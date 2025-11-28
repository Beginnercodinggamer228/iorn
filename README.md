![logo](/assets/iorn_ico_logo.png)
# IORN Programming Language

IORN is a modern programming language with intuitive syntax and Russian localization support.

[githubIorn](https://github.com/Beginnercodinggamer228/iorn)

> **WARNING** - This language is still in **development**, so there may be bugs!

## 🚀 Installation

1. Download the release
2. Restart VS Code

> What should I do if I can't compile the code?
> 1. Go to the official website [github](https://github.com/Beginnercodinggamer228/iorn) project and download `iorn.exe`
> 2. Transfer the file to your project
>   - For convenience, after you have transferred `iorn.exe` to the project, transfer it to **Programm Files** - `C:\Program Files`
>   - Write down the commands - 
>     ```powershell
>     $PATH = [Environment]::GetEnvironmentVariable("PATH") 
>     $my_path = "C:\Program Files\iorn.exe"
>     [Environment]::SetEnvironmentVariable("PATH", "$PATH;$my_path", "User")
>     ```
>     - Check by writing the command -
>     ```bash
>     iorn YourScript.iorn
>     ```
> .

## 📝 Syntax

### Comments
```iorn
## Single line comment
@rem Batch-style comment

@remLine(
    Multi-line
    comment
)

"""
Block comment
Python-style
"""
```

### Variables

#### Variable Declaration
```iorn
new variable name type = value;
```

#### Variable Modification
```iorn
rename variable name type = new_value;
```

#### NULL Variables
```iorn
new variable name NULL;
```

**Data Types:**
- `numeric` - integers
- `floating` - floating point numbers
- `string` - strings
- `boolean` - TRUE/FALSE

**Examples:**
```iorn
new variable V1 numeric = 15;
new variable pi floating = 3.14;
new variable text string = "Hello World";
new variable flag boolean = TRUE;
```

### Module Import
```iorn
import terminal.*;           ## Import all functions
import terminal.Print;       ## Import only Print
import terminal.input;       ## Import only input
```

### Conditional Statements
```iorn
if (condition) then:
    ## code
else to if (condition) resume:
    ## code
else perform:
    ## code
endif;
```

### Comparison Operators
- `==` - equal
- `!=` - not equal
- `>=` - greater than or equal
- `<=` - less than or equal
- `>` - greater than
- `<` - less than

### Mathematical Operators
- `**` - exponentiation
- `/` - division
- `*` - multiplication
- `%` - modulo
- `+` - addition
- `-` - subtraction

### String Interpolation
```iorn
Print(f"$[variable] text");
```

### Input/Output
```iorn
Print("text");                           ## output
new variable input_var string = input("prompt: ");  ## input
```

### Special Commands
- `ignore` - skip code block
- `resume` - continue execution (in else to if)

## 📁 Examples

### Hello World
```iorn
import terminal.*;

new variable hiVar string = input("Hello - ");

if (hiVar == "Hello") then:
    Print(f"You:'$[hiVar]', :3");
else perform:
    Print(f"You:'$[hiVar]', I don't know... But I hope it was nice ^_^");
endif;
```

### Mathematical Operations
```iorn
import terminal.*;

new variable V1 numeric = 15;
new variable V2 numeric = 3;

if (V1 * V2 == 45) then:
    Print(f"$[V1] * $[V2] = 45");
else perform:
    Print(f"$[V1] * $[V2] = $[V1 * V2]");
endif;
```

## 🛠️ Compilation and Execution

### Regular Execution
```bash
iorn filename.iorn
```

### Packaging to Executable
```bash
iorn filename.iorn --package=.exe --out_name=myapp --loop_main=True
```

**Packaging Parameters:**
- `--package=.exe` - output file extension
- `--out_name=name` - output file name
- `--icon=path` - application icon
- `--loop_main=True/False` - wait for Enter before closing

## 🎨 Editor Support

- ✅ VS Code - syntax highlighting and icons
- ✅ File icons in Windows Explorer

## 📋 Features

- Russian localization of keywords
- Intuitive syntax
- Unicode support (including Russian input/output)
- Built-in terminal operations
- String interpolation with `$[variable]`
- Mathematical expressions in variables
- Syntax checking with detailed errors
- Packaging to executables
- Support for 4 types of comments
- Data type validation on input
