package main

import (
	"bufio"
	"crypto/sha1"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"log"
	"os"
	"sort"
	"strings"
	"text/template"
	"time"

	"github.com/gomarkdown/markdown"
)

type twtxtItem struct {
	Date     time.Time
	Contents string
}

type tmplCtx struct {
	LastUpdateTime time.Time
	Items          []twtxtItem
}

func (t twtxtItem) FormattedDate(layout string) string {
	return t.Date.Format(layout)
}

func (t twtxtItem) Hash() string {
	hasher := sha1.New()
	hasher.Write([]byte(t.Contents))
	timeBytes := make([]byte, 8)
	binary.LittleEndian.PutUint64(timeBytes, uint64(t.Date.Unix()))
	hasher.Write(timeBytes)
	return hex.EncodeToString(hasher.Sum(nil))
}

func (t twtxtItem) FormattedTxt() string {
	return string(markdown.ToHTML([]byte(t.Contents), nil, nil))
}

func (t tmplCtx) FormattedUpdateTime(layout string) string {
	return t.LastUpdateTime.Format(layout)
}

func main() {
	log.SetFlags(log.Flags() &^ (log.Ldate | log.Ltime))

	if len(os.Args) != 4 {
		fmt.Println("usage: twtxt-tmpl <twtxt-feed> <template> <output>")
		os.Exit(1)
	}

	feed, err := os.Open(os.Args[1])
	if err != nil {
		log.Fatal(err)
	}
	defer feed.Close()

	tmpl, err := template.ParseFiles(os.Args[2])
	if err != nil {
		log.Fatal(err)
	}

	out, err := os.Create(os.Args[3])
	if err != nil {
		log.Fatal(err)
	}
	defer out.Close()

	var ctx tmplCtx
	ctx.LastUpdateTime = time.Unix(0, 0)
	scanner := bufio.NewScanner(feed)
	lineno := 0
	for scanner.Scan() {
		lineno++
		line := scanner.Text()

		if len(line) == 0 || line[0] == '#' {
			continue
		}

		date, contents, success := strings.Cut(line, "\t")
		if !success {
			log.Fatal("could not find a tab character on line ", lineno)
		}

		var item twtxtItem
		item.Contents = contents

		item.Date, err = time.Parse(time.RFC3339, date)
		if err != nil {
			log.Fatal("error parsing date on line ", lineno, ": ", err)
		}

		if item.Date.After(ctx.LastUpdateTime) {
			ctx.LastUpdateTime = item.Date
		}

		ctx.Items = append(ctx.Items, item)
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	sort.Slice(ctx.Items, func(i, j int) bool {
		return ctx.Items[i].Date.After(ctx.Items[j].Date)
	})

	err = tmpl.Execute(out, ctx)
	if err != nil {
		log.Fatal(err)
	}
}
