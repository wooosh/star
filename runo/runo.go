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
// TODO: ordered lists
// TODO: definition lists

// TODO: line number in error output

var body string

func write(txt string) {
	body += txt
}

func writeRune(b rune) {
	body += string(b)
}

func tryToggleTag(tag string, c rune, prev *rune, status *bool) {
	if *status {
		write("</" + tag + ">")
		*status = false
	} else if *prev == ' ' {
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
	prev := ' '
	for _, c := range txt {
		switch c {
		case '`':
			tryToggleTag("code", c, &prev, &code)
		case '*':
			if !code {
				tryToggleTag("strong", c, &prev, &bold)
			} else {
				writeRune('*')
				prev = '*'
			}
		case '/':
			if !code {
				tryToggleTag("em", c, &prev, &italic)
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
	prevListDepth := 0

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()

		if len(line) < 1 {
			continue
		}

		if line[0] == '-' {
			count := strings.IndexFunc(line, func(c rune) bool { return c != '-' })

			if prevListDepth == count {
				write("</li>")
			}

			if prevListDepth == 0 {
				write("<ul>")
				prevListDepth++
			}

			for ; prevListDepth < count; prevListDepth++ {
				write("<ul>")
			}

			for ; prevListDepth > count; prevListDepth-- {
				write("</li></ul></li>")
			}

			write("<li>")
			writeInlineMarkup(line[count:])
			continue
		} else if prevListDepth != 0 {
			for ; prevListDepth > 0; prevListDepth-- {
				write("</li></ul>")
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

	if prevListDepth != 0 {
		for ; prevListDepth > 0; prevListDepth-- {
			write("</li></ul>")
		}
	}

	err = t.Execute(os.Stdout, doc{title, body})
	if err != nil {
		log.Fatal(err)
	}
}
