# mio

Mio is a simple, dependency-free text editor inspired by nano and kilo. My goal is for mio to be easy to pick up and use, easy to compile, and easy to extend.

Currently, only single buffer editing is supported, but multiple buffers are being considered.

## Supported Languages

The following languages currently have some amount of syntax highlighting in mio:
* C/C++
* HTML
* Fish
* Javascript
* MUMPS
* Ruby

## Planned Language Support

The following languages are planned for support in mio:
* Crystal
* CSS
* Haskell
* Objective-C
* PHP
* Python
* Scheme
* Swift
* Vimscript

## Keybindings
| Key           | Function          |
| ------------- |:-----------------:|
| ^Q            | Quit              |
| ^S            | Save              |
| ^F            | Find              |
| ^B            | Beginning of line |
| ^E            | End of line       |
| ^G            | Go to line        |
| PgDn          | Scroll down       |
| PgUp          | Scroll up         |

## Installation
```
git clone https://github.com/spencerking/mio.git
cd mio
make
```

## Contributing
1. Open an issue to discuss your feature
2. Fork the repo (<https://github.com/spencerking/mio/fork>)
3. Create your feature branch (`git checkout -b my-new-feature`)
4. Commit your changes (`git commit -am 'Add some feature'`)
5. Push to the branch (`git push origin my-new-feature`)
6. Create a new Pull Request

## Special Thanks
* [antirez](https://github.com/antirez)
