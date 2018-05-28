
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>



int childExitMethod=-5;
int tracker; 
int background; 
int foreground; 
int currentFG;
int stopBackground=0;
int blankOrComment; 
int sigSTP;
int dollarSign;
int blank;
int bStatus; 
int bgCounter=0;

//CITE: CS344 Brewster Class Lectures Slides Block 3


void  exitMessage(int status)

{

//If process was terminated by a signal

if(WIFSIGNALED(status) !=0)
{
//get signal number
int termSignal= WTERMSIG(status);

printf ("The process was terminated by signal %d. \n", termSignal);
fflush (stdout);

}

//if process did not terminate by signal
else if(WIFEXITED(status))
{
int exitStatus=WEXITSTATUS(status);

//print out exit status
printf("Exited. Status was %d \n", exitStatus);
fflush(stdout);

}


}



void execute(char** argv)
{

if(execvp(*argv,argv)<0)
{
//if error with execution
perror("Exec failure!");

exit(1);
}

}



//CITE: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
char *replaceWord(const char *s, const char *oldW,
                                 const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
 
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
 
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
 
    // Making new string of enough length
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);
 
    i = 0;
    while (*s)
    {
        // compare the substring with the result
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
 
    result[i] = '\0';
    return result;
}



//sigstop handler
void  catchSIGSTP(int signo)
{

if (stopBackground==0)
{
//make stop background to 1 so it CAN stop all bg processes
char *message= "Processes will now all be foreground.\n";
write(STDOUT_FILENO, message, 38);
fflush(stdout);

stopBackground=1;
}

else

{
//make background proceses resume as normal 
char *message2 = "Background processes now allowed.\n";
write(STDOUT_FILENO,message2, 34);
fflush(stdout);

stopBackground=0;


}

}


int main()
{

//init background processes array with -1 
int bg[100];

int g;
for(g=0; g<100; g++)
{
bg[g]=-1;

}


//initialize struct
struct sigaction SIGINT_v2= {0}; 
//parent ignores SIGINT
SIGINT_v2.sa_handler=SIG_IGN;

sigaction(SIGINT, &SIGINT_v2,NULL);



struct sigaction BGSTOP = {0};

BGSTOP.sa_handler=catchSIGSTP;
sigfillset(&BGSTOP.sa_mask);
//tell system to automatically restart
BGSTOP.sa_flags= SA_RESTART;
sigaction(SIGTSTP, &BGSTOP, NULL);


char* arguments[512];
int args=0; 

char *newString=NULL;

int foreground=0;
dollarSign=0; //this is for the $$ pid part 

//Cite: https://oregonstate.instructure.com/courses/1674426/pages/3-dot-3-advanced-user-input-with-getline

	int numCharsEntered = -5; // How many chars we entered
	int currChar = -5; // Tracks where we are when we print out every char
	size_t bufferSize = 0; // Holds how large the allocated buffer is
	char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
	


	while(1)
	{
		// Get input from the user
		while(1)	
		{

			printf(":");
			fflush(stdout);

			numCharsEntered = getline(&lineEntered, &bufferSize, stdin); // Get a line from the user
			if (numCharsEntered == -1)
				clearerr(stdin);
			else
				break; // Exit the loop - we've got input
		}

		lineEntered[strcspn(lineEntered, "\n")] = '\0'; // Remove the trailing \n that getline adds
//		printf("Here is the cleaned line: \"%s\"\n", lineEntered);
char original[200], linev2[300], tokenUse[300];



strcpy(original, lineEntered); 		


int comment=0;
	
char* linePointer =lineEntered;

char a= linePointer[0];
char b= '#';


//check if line begins with a comment by just looking at the first character

if (a==b)

{
comment=1;
}


//control input here
int length= strlen(lineEntered); //get length of line entered 

if (length==0 )
//do nothing if you have a blank line or comment 
{
//line entered was a blank
blank=1;
}
 

//proceed if command entered was neither a comment or blank
if (!comment && !blank)
{

//Check to see if there is a & at the end of the command if there is set background  flag. 


char *backgroundTok = strtok(lineEntered, " "); 

int count=0;

while (backgroundTok !=NULL) 
{

//keep advancing to see if & is in the end 

if  (strcmp(backgroundTok,"&")==0 )
{
background=1;

}

else
{
background=0;
}

backgroundTok =strtok(NULL, " ");


count++;


//the status of background at the very end will determine whether a & exists at the end

}


//stopBackground prevents any background process from running  for SIGSTP signal; this is ignored otherwise

if(stopBackground)
{
background=0;
}

//scan the line entered and see if there are any $$ if there is replace them with pid 
int pid= getpid();

char convert[20];
//convert pid to string 
sprintf(convert, "%d", pid);
char c[]="$$";
//replaceWord function will replace any instance of $$ with pid 
newString=replaceWord(original,c,convert);





//run strtok on the new string that has any instances of $$ replaced with pid 

char *token = strtok(newString, " "); 


//BUILT IN COMMANDS 

if (strcmp(token,"exit")==0)
{
//kill bg processes before exit
int j;
for (j=0;j<100;j++)
{

if (bg[j]>0)
{
kill(bg[j],SIGKILL);

}
}

exit(0);
}

//if you enter just cd without any arguments 
else if (strcmp(token,"cd")==0)
{
//advance token to see if there is another path specified after cd

char *next=strtok(NULL," ");
//there is no directory specified after cd 
if (next==NULL || strcmp(next,"&")==0)
{//go to home directory
	chdir(getenv("HOME")); 
}

else 
{
//else take them to the new directory 
chdir(next);
continue;
}

//end of cd if statement block 
}


else if (strcmp(token,"status")==0)
{

//print exit message of how last process exited
exitMessage(childExitMethod);
continue; 
}


else


//if not built in function 

{

//blanks and comments excluded because you don't want further processing 

//{

int bOutputFile =0; 
char *outputFile;
char *inputFile; 
int bInputFile=0;


//if the background flag is 1 at the very end that means & was in last position

//get all the other stuff 

int count2=0;

//iterate through the entire string using the space as a delimiter
while(token !=NULL) 
{
count2++;


if(strcmp(token,">")==0)
{

//the word right after > is the output file name 
outputFile= strtok(NULL, " "); 
//advance token after getting the output file name
token=strtok(NULL, " "); 
//see output flag to 1 
bOutputFile=1; 


if(token == NULL)
break;

}


if(strcmp(token,"<")==0)

{
//the word right after < is the inputfile name
inputFile= strtok(NULL, " ");
//advance the token
token= strtok(NULL, " ");
//set intput flag to true to indicate there is an input file 
bInputFile=1;

if(token == NULL)
break;

}


if(strcmp(token,"&")==0)
{

//Case 1: & is at the end of string but tis token & is not the one on the end, Case2: background & is  not at the end and SIGSTP not activated
//Case3: SGSTP is on and  the & is in the middle of the string 
if((background==1 && count2< count) || (background ==0 && !stopBackground)||( stopBackground && count2<count))
{
//add the & to the args since it's in the line entered but not at the end of it 
arguments[args]=strdup(token);
args++;

token=strtok(NULL, " ");
//advance the token 


if (token== NULL)
break; 

}

//the & exists at the end
else 
{
break;
}

}




//for everything else and to advance the token 

else

{
//make sure you're not relooking at a  token that has values > < or &

if((strcmp(token,">")!=0) && (strcmp(token,"<")!=0)&& (strcmp(token,"&")!=0))
{
//put the new value in arguments array
arguments[args]=strdup(token);
args++;

token=strtok(NULL, " ");
if(token == NULL)
break;

}

}


//end of token while loop 
}

//add null to end of argument 
arguments[args]=NULL; 




/*
int j;
for (j=0; j<args;j++)
{
printf("arguments are %s\n",arguments[j]);
}
*/

//FORK HAPPENS HERE


int bpid2;

pid_t spawnpid = -5;

spawnpid= fork();

//Switch statement begins 
	
 switch(spawnpid)
	{


//if error
case -1:

perror("Error Switch Statement Case 0");
exit(1);

break;



//CHILD CASE


int sourceFD,result, targetFD, finput, foutput;

case 0:


if (!background)
{
currentFG= getpid();

//sigint behavior for child. will do default behavior 

SIGINT_v2.sa_handler=SIG_DFL;
sigaction(SIGINT,&SIGINT_v2, NULL); 

}
if(background)
{
//get background process id 
 bpid2=getpid();
}


//CITE:CS344  Lecture Slides on redirection

if(bInputFile)

{
//open file so that's it's read only
sourceFD= open(inputFile, O_RDONLY);
//if error in opening file 
if (sourceFD ==-1)
{
perror ("source open()");
fflush(stdout);
exit(1);
}

//have stdin point to sourceFD 
result= dup2(sourceFD,0);

//if dup2 error results
if(result==-1)
{

perror("source dup2()");

fflush(stdout);
exit(2);
}

close(sourceFD); 

//end of input file bracket
}



//output file 

if(bOutputFile)

{
//create a file to write into it
targetFD=open(outputFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);

//if error opening file
if (targetFD==-1)
{

perror("target open()");
fflush(stdout);
exit(1);


}

//make stdout point to targetFD
result= dup2(targetFD,1);

//if dup2 error
if(result ==-1)

{

perror("target dup2()");
fflush(stdout);
exit(2);

}


close(targetFD); 
}



//redirected background input and output to dev/null
	
if(background)
		 {

//if not background input file
		   if(!bInputFile)
		   {
//redirect stdin to dev/null
		      finput=open("/dev/null",O_RDONLY);
//if error in opening file 
          if(finput==-1)
					{perror("source open()");
					fflush(stdout);  
          exit(1);
					}					
//point stdin to finput
					int result2= dup2(finput,0);
//if error with dup2

		      if(result2==-1)
						{
						perror("source dup2()");
						fflush(stdout);
						exit(2);

 						}


					close(finput);
	        }

//if no background  outfile  file 
		   if(!bOutputFile)
		   {
//redirect output to dev/null
		      foutput=open("/dev/null",O_WRONLY);

//if open results in error
					if (foutput==-1)
				{
					perror("target open()");
					fflush(stdout);
					exit(1);

				}


				//make stdout point to foutput 
		    int result3=  dup2(foutput,1);
				if (result3==-1)
				{
//if dup2 results in error 

					perror("target dup2()");
					fflush(stdout);
					exit(2);

				}


					close(foutput);
		   }

		 }

//execute arguments using execute function 

execute(arguments);

break;




default:

//in parent 

if (background)
{

//print out background process id when in parent
printf("background process starts pid: %d \n", spawnpid);
fflush(stdout);

}




if (!background)

{


//parent is waiting for foreground process to end
int  wait= waitpid(spawnpid, &childExitMethod,0);
//foreground process status print out 

if (WIFSIGNALED(childExitMethod)!=0)
{
//if foreground process was terminated by a signal 
int termSignal= WTERMSIG(childExitMethod);

printf("Terminated by signal  %d \n", termSignal);
fflush(stdout);

}

}


break;
//end of switch statement
	}  


//empty out all the arguments before while loop ends 

int i; 
for (i = 0; i < args; i++)
		{
			arguments[i] = NULL;
		}
//reset variables 
args=0;
bInputFile=0;
bOutputFile=0;
background=0;
currentFG=0;


}//end of else
}//end of if statemnet for comment and blanks 


//periodically check background processes


int bpid;

//keep checking for live background processes if they're there

bpid= waitpid(-1, &bStatus, WNOHANG);
//while the background process id is getting returned 

while  (bpid >0)

{
bg[bgCounter]=bpid;
bgCounter++;

//print out how it terminated 
if(WIFSIGNALED(bStatus)!=0 || WIFEXITED(bStatus))
{

printf("Background process terminated. Process id: %d \n", bpid);fflush(stdout);
exitMessage(bStatus);
}
//see if there's another background process that terminated 

bpid= waitpid(-1, &bStatus, WNOHANG);
} 


//reset blank and comment variables 

comment=0;
blank=0;


//end of getline while loop
	}

return 0; 
}





