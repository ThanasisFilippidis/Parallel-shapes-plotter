A plotter that is using fork to create parallel workers that communicate with the main proccess through named pipes synched with the use of poll function in order to finally plot multiple shapes using gnu plot.

Readme

Thanasis Filippidis sdi1400215@di.uoa.gr

## Compile

To compile run `make`

## Run

Command line options:
 * -i binary file: filename of input file 
 * -w workers: integer number of workers
 * -d directory: directory name to save files

Invocation:

    ./shapes -i binary file -w workers -d directory
    
    in all possible combinations 


## Implementation Notes


### Shapes

For the shapes implementation, the CLI I get the whole line with input string function
and then I break it with strtok and save it in an array which contains the shapes and
the arguments and then I break every set of arguments in another array so that I can
get the arguments one by one. I used the function execlp for the workers. For the named 
pipes I followed the notes provided by the instructor step by step and to read and write
from the pipes and sync them I used the poll function. At the end of every CLI I wait 
for all the handlers to finish and then I create the gnuplot, following the given
instructions. The CLI input ends when I give the command "end;".

### Utilities

All the utilies use the equations of the links given.
