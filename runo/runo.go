package main

import (
	"bufio"
	"html"
	"log"
	"os"
	"regexp"
	"strings"
	"text/template"
)

// TODO: set
// TODO: setblock
// TODO: error out on unbalanced inline markup
// TODO: definition lists

// TODO: line number in error output

var body string
var listPath []string

func write(txt string) {
	body += txt
}

func writeRune(b rune) {
	body += string(b)
}

func tryToggleTag(tag string, c rune, prev *rune, next rune, status *bool) {
	if *status {
		write("</" + tag + ">")
		*status = false
	} else if *prev == ' ' && next != ' ' {
		write("<" + tag + ">")
		*status = true
	} else {
		writeRune(c)
	}
	*prev = c
}

var linkPattern = regexp.MustCompile(`\[(.*?)\]\((.*?)\)`)

func writeInlineMarkup(txt string) {
	var bold, italic, code bool

	txt = html.EscapeString(txt)
	txt = linkPattern.ReplaceAllString(txt, "<a href='${2}'>${1}</a>")
	txt = strings.ReplaceAll(txt, "---", "&mdash;")
	txt = strings.ReplaceAll(txt, "--", "&ndash;")
	// TODO: fix prev mess
	prev := ' '
	for i, c := range txt {
		next := ' '
		if i+1 < len(txt) {
			next = rune(txt[i+1])
		}
		switch c {
		case '`':
			tryToggleTag("code", c, &prev, next, &code)
		case '*':
			if !code {
				tryToggleTag("strong", c, &prev, next, &bold)
			} else {
				writeRune('*')
				prev = '*'
			}
		case '/':
			if !code {
				tryToggleTag("em", c, &prev, next, &italic)
			} else {
				writeRune('/')
				prev = '/'
			}
		default:
			prev = c
			writeRune(c)
		}
	}
}

func readBlock(scanner *bufio.Scanner) string {
	if !scanner.Scan() {
		if err := scanner.Err(); err != nil {
			log.Fatal(err)
		} else {
			log.Fatal("unexpected EOF when reading fence definition of block")
		}
	}

	fence := scanner.Text()
	block := ""

	for scanner.Scan() {
		line := scanner.Text()
		if line == fence {
			return block
		}
		block += line + "\n"
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	log.Fatal("could not find ending fence '" + fence + "' of block")
	return ""
}

func requireArg(directive string, hasArg bool, isRequired bool) {
	if hasArg != isRequired {
		if isRequired {
			log.Fatal(directive + " requires an argument")
		} else {
			log.Fatal(directive + " must not have an argument")
		}
	}
}

type doc struct {
	Title string
	Body  string
}

func pushListPath(tag string) {
	listPath = append(listPath, tag)
}

func popListPath() string {
	tag := listPath[len(listPath)-1]
	listPath = listPath[:len(listPath)-1]
	return tag
}

func writeListItem(listChar rune, listTag string, line string) {
	count := strings.IndexFunc(line, func(c rune) bool { return c != listChar })

	if len(listPath) == count {
		write("</li>")
	}

	if len(listPath) == 0 {
		write("<" + listTag + ">")
		pushListPath(listTag)
	}

	for ; len(listPath) < count; pushListPath(listTag) {
		write("<" + listTag + ">")
	}

	for len(listPath) > count {
		write("</li></" + popListPath() + "></li>")
	}

	write("<li>")
	writeInlineMarkup(line[count:])
}

func main() {
	if len(os.Args) != 3 {
		log.Fatal("expected one template filename followed by a runo filename")
	}

	t, err := template.ParseFiles(os.Args[1])
	if err != nil {
		log.Print(err)
		return
	}

	file, err := os.Open(os.Args[2])
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var title string

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()

		if len(line) < 1 {
			continue
		}

		if line[0] == '-' {
			writeListItem('-', "ul", line)
			continue
		} else if line[0] == '#' {
			writeListItem('#', "ol", line)
			continue
		} else if len(listPath) != 0 {
			for len(listPath) > 0 {
				write("</li></" + popListPath() + ">")
			}
		}

		if line[0] != '.' {
			write("<p>")
			writeInlineMarkup(line)
			write("</p>")
			continue
		}

		directive, arg, hasArg := strings.Cut(line[1:], " ")
		arg = strings.TrimSpace(arg)
		switch directive {
		case "title":
			requireArg(directive, hasArg, true)
			title = arg
		case "h1", "h2", "h3", "h4", "h5":
			requireArg(directive, hasArg, true)
			write("<" + directive + ">")
			writeInlineMarkup(arg)
			write("</" + directive + ">")
		case "hr":
			requireArg(directive, hasArg, false)
			write("<hr>")
		case "code":
			// TODO: handle arg
			write("<pre><code>")
			write(html.EscapeString(readBlock(scanner)))
			write("</code></pre>")
		case "quote":
			write("<figure class='quote'><blockquote>")
			writeInlineMarkup(readBlock(scanner))
			write("</blockquote>")
			if hasArg {
				write("<figcaption>&mdash;")
				writeInlineMarkup(arg)
				write("</figcaption>")
			}
			write("</figure>")
		default:
			log.Fatal("unknown directive '" + directive + "'")
		}
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	if len(listPath) != 0 {
		for len(listPath) > 0 {
			write("</li></" + popListPath() + ">")
		}
	}

	err = t.Execute(os.Stdout, doc{title, body})
	if err != nil {
		log.Fatal(err)
	}
}
