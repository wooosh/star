package main

import (
	"bufio"
	"fmt"
	"html"
	"io"
	"log"
	"os"
	"strings"
)

const (
	ClassColorBase = 'a'
	ClassBold      = 'z'
	ClassItalic    = 'Z'
	ClassUnderline = 'x'
)

var (
	bold      = false
	italic    = false
	underline = false
	fg        = -1
	bg        = -1
)

func reset() {
	bold = false
	italic = false
	underline = false
	fg = -1
	bg = -1
}

func badCode(code string) {
	log.Fatal("invalid escape code '", code, "'")
}

func getColor(color byte, offset int) int {
	if '0' <= color && color <= '8' {
		return int(color-'0') + offset
	} else if color == '9' {
		return -1
	}

	log.Fatal("invalid color '", color, "'")
	return -1
}

func main() {
	r := bufio.NewReader(os.Stdin)

	for {
		// read and print escaped text until next escape code
		txt, err := r.ReadString(0x1b)
		if err == io.EOF {
			fmt.Print(html.EscapeString(txt))
			break
		} else if err != nil {
			log.Fatal(err)
		}
		fmt.Print(html.EscapeString(txt[:len(txt)-1]))
		// handle escape code
		code, err := r.ReadString('m')
		if err == io.EOF {
			log.Fatal("unterminated or unknown escape code present")
		} else if err != nil {
			log.Fatal(err)
		}

		if code[0] != '[' {
			log.Fatal("expected [ after escape character")
		}

		// remove opening bracket and terminating m
		cmds := strings.Split(code[1:len(code)-1], ";")
		for _, cmd := range cmds {
			if len(cmd) == 0 || cmd[0] == '0' {
				bold = false
				italic = false
				underline = false
				fg = -1
				bg = -1
				continue
			}

			switch cmd[0] {
			case '1':
				if len(cmd) == 1 {
					bold = true
				} else {
					badCode(cmd)
				}
			case '2':
				if len(cmd) == 2 {
					switch cmd[1] {
					case '2':
						bold = false
					case '3':
						italic = false
					case '4':
						underline = false
					default:
						badCode(cmd)
					}
				} else {
					badCode(cmd)
				}
			case '3':
				if len(cmd) == 1 {
					italic = true
				} else if len(cmd) == 2 {
					fg = getColor(cmd[1], 0)
				} else {
					badCode(cmd)
				}
			case '4':
				if len(cmd) == 1 {
					underline = true
				} else if len(cmd) == 2 {
					bg = getColor(cmd[1], 0)
				} else {
					badCode(cmd)
				}
			}
		}

		next, err := r.Peek(1)
		if err == nil && next[0] == 0x1b {
			continue
		}

		classes := ""
		classCount := 0
		if fg != -1 {
			classes += string(ClassColorBase+fg) + " "
			classCount++
		}
		if bg != -1 {
			classes += "b" + string(ClassColorBase+bg) + " "
			classCount++
		}
		if bold {
			classes += string(ClassBold) + " "
			classCount++
		}
		if italic {
			classes += string(ClassItalic) + " "
			classCount++
		}
		if underline {
			classes += string(ClassUnderline) + " "
			classCount++
		}

		if len(classes) > 1 {
			classes = classes[:len(classes)-1]
			if classCount == 1 {
				fmt.Print("<p class=", classes, ">")
			} else {
				fmt.Print("<p class='", classes, "'>")
			}
		} else {
			fmt.Print("<p>")
		}
	}
}
