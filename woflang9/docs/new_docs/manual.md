# WofLang User Guide  

## Program Structure

/build # Compiled binaries
/src # Core source code
/modules # Plugins and extensions
/docs # Documentation
/tests # Test suite

shell
Copy
Edit

## Using the REPL

Example Session:
42 ⬆
π × 2
SacredGeometry invoke

markdown
Copy
Edit

## Modules

- **Symbolic Math**: Core math with Unicode operations  
- **Units System**: Physical unit computation  
- **Fractal Plugin**: Future fractal algebra support  
- **Markov AI**: Math suggestion engine  

Installing Modules:
- Drop DLL into `/modules`
- WofLang auto-loads on next run

Creating Your Own:
- Build C++ shared library
- Follow module interface template in `/docs/dev_modules.md`