#include <unistd.h>     // getpid(), getcwd()
#include <sys/types.h>  // type definitions, e.g., pid_t
#include <sys/wait.h>   // wait()
#include <signal.h>     // signal name constants and kill()
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h> //for open/close
#include <fcntl.h> // for open/close

using namespace std;
FILE *input=NULL;
int batch = 0;
int endSh=0;
char* argv[100];
int bg = 0;

void batchParse(char* line);
void execute(char* cmd, char *argv[],int argn);
void parse(char* line);
void setNull();
void multipipe(char *cmd,char* args[], int argn);

int main(int argc,char *argv[]){
    if(!endSh){
        if(argc==1&&!endSh){ //interactive mode
            while(true&&!endSh){
                setNull(); //set argv to null
		string cmdLine;
                printf("myshell> ");
		if(getline(cin,cmdLine)==0){endSh=1;}
                char *cmd = new char[cmdLine.length()+1];
                strcpy(cmd,cmdLine.c_str());
		if((cmd!=" ")&&(cmd!="\n")&&(!endSh)){
			setNull();
                	batchParse(cmd);
		}
            }
        }
	else if((argc==2)&&!endSh){ //batch mode
            //TODO: errors in batch mode must stop code and print “Error: command exited with non-zero exit code”
            batch = 1;
            ifstream file(argv[1]);
            string nextinst;
            while(getline(file,nextinst)&&!endSh){
                //parse command
		char *inst = new char[nextinst.length()+1];
		strcpy(inst,nextinst.c_str());
		if((inst!="")&&(inst!=" ")&&(inst!="\n")&&(inst!="\t")&&(inst!="\0")){
                	batchParse(inst);
		}
            }
            argc=0;
         }
	else if(endSh){ return 0; }
	else{
		cout<< "Invalid number of arguments passed" << endl;
		return -1;
	}
    }
  return 0;
}

void setNull(){
    for(int i=0; i<100; i++){
        argv[i]=NULL;
    }
}

//parses each line into commands
void batchParse(char* line){ //seperate commands based on ; and new lines
        char *p = strtok(line, ";\n");
        while (p) {
                if((p!=" ")&&(p!="\n")&&(p!="\0")){
			if(batch){cout << "Command: " << p << endl;} //TODO: USE FFLUSH TO PRINT ASAP(or not because thats dumb
			parse(p);
                }
                p = strtok(NULL, ";\n");
        }
}

//parses each command into the command and arguments
void parse(char* line){ //seperate commands and arguments based in whitespace
    int i = 0;
    while(line[i]!='\0'){
	if(line[i]=='&'){bg=1;line[i]='\0';}
	i++;
    }
    string buf; // Have a buffer string
    stringstream ss(line); // Insert the string into a stream
    vector<char*> tokens; // Create vector to hold our words
    while (ss >> buf){
	char *temp = new char[buf.length()+1];
        strcpy(temp,buf.c_str());
        tokens.push_back(temp);
    }
    char** argv = new char*[tokens.size()+1]; //set char array to hold args
    for ( int k = 0; k < tokens.size(); k++ ){ //TODO: EXECUTE COMMANDS
      	argv[k] = tokens[k];
    }
	argv[tokens.size()] = NULL;
	execute(argv[0],argv, tokens.size());
	bg=0;
}

//executes commands
void execute(char* cmd, char *args[],int argn){ //TODO: EXECUTE JOBS, deal with background processes
//argn = # of arguments and the command, so ls -a would be argn 2, just as argv[0] is ls and argv[1] is -a
    int piping = 0;
    for(int i=0; i<argn; i++){
	if(strcmp(args[i],"|")==0){piping=1;}
    }
    if(strcmp(args[argn-1],"&")==0){
	bg=1;
    }
    if((strcmp(cmd,"exit")==0)||(strcmp(cmd,"quit")==0)){ //built-in command quit
      endSh=1;
    }
    else if(piping){
	for(int i=0; i<argn; i++){
		if(strcmp(args[i],"|")==0){
               		args[i] = NULL;
                        char* right[argn-i];
                        int c = 0;
                        for(int j=i; j<argn-1; j++){
	                        right[j-i]=args[j+1];
                                c++;
                        }
                        right[c]=NULL;
                        int p[2];
                        pipe(p);
                        pid_t lpid = fork();
                        if(lpid==0){//left (child)
				dup2(2,1);
                        	dup2(p[1],STDOUT_FILENO);
                                execvp(cmd,args);
                        }
                        else{//right (parent)
				pid_t rpid = fork();
				close(p[1]);
				if(rpid == 0){ //right child
					dup2(2,1);
                                	dup2(p[0],STDIN_FILENO);
                                	multipipe(right[0],right,c);
					//close(p[1]);
				}
				else{ //parent
					if(!bg){waitpid(rpid,0,0);}
				}
                        }
                        break;
		}
	}
    }
    else{
          if(strcmp(cmd,"cd")==0){ //built-in command cd
              if(args[1]==NULL){
                  chdir("/");
              }
              else{
                  chdir(args[1]);
              }
          }
          else{ //standard forking process
      		pid_t childID = fork();
      		if(childID<0){
        		perror("Error when forked");
        		endSh=1;
      		}
      		else if(childID==0){ //Child process
                        for(int i=0; i<argn; i++){
                                if(strcmp(args[i],">")==0){
                                        int newstdout = open(args[i+1],O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
                                        close(1);
                                        dup(newstdout);
                                        close(newstdout);
                                        args[i]=NULL;
                                }
                                else if(strcmp(args[i],"<")==0){
                                        int newstdin = open(args[i+1],O_RDONLY);
                                        close(0);
                                        dup(newstdin);
                                        close(newstdin);
                                        args[i]=NULL;
                                }
                        }
        		execvp(cmd,args);
        		perror(cmd); //only happens when there is an error
        		endSh=1;
      		}
      		else{
			if(!bg){ //if not a background process, wait for it
        			if (waitpid(childID,0,0)<0){//parent process
          				endSh=1;
	  				perror("Error when waiting for child");
        			}
			}
      	  	}
    	  }
     }
}





void multipipe(char *cmd,char* args[], int argn){
    int piping = 0;
    for(int i=0; i<argn; i++){
        if(strcmp(args[i],"|")==0){piping=1;}
    }
    if(strcmp(args[argn-1],"&")==0){
        bg=1;
    }
    if((strcmp(cmd,"exit")==0)||(strcmp(cmd,"quit")==0)){ //built-in command quit
      endSh=1;
    }
    else if(piping){
        for(int i=0; i<argn; i++){
                if(strcmp(args[i],"|")==0){
                        args[i] = NULL;
                        char* right[argn-i];
                        int c = 0;
                        for(int j=i; j<argn-1; j++){
                                right[j-i]=args[j+1];
                                c++;
                        }
                        right[c]=NULL;
                        int p[2];
                        pipe(p);
                        pid_t lpid = fork();
                        if(lpid==0){//left (child)
                                dup2(2,1);
                                dup2(p[1],STDOUT_FILENO);
                                execvp(cmd,args);
                        }
                        else{//right (parent)
                                pid_t rpid = fork();
                                close(p[1]);
                                if(rpid == 0){ //right child
                                        dup2(2,1);
                                        dup2(p[0],STDIN_FILENO);
                                        multipipe(right[0],right,c);
                                        //close(p[1]);
                                }
                                else{ //parent
                                        if(!bg){waitpid(rpid,0,0);}
                                }
                        }
                        break;
                }
        }
    }
    else{
          if(strcmp(cmd,"cd")==0){ //built-in command cd
              if(args[1]==NULL){
                  chdir("/");
              }
              else{
                  chdir(args[1]);
              }
          }
          else{ //standard forking process
                pid_t childID = fork();
                if(childID<0){
                        perror("Error when forked");
                        endSh=1;
                }
                else if(childID==0){ //Child process
                        for(int i=0; i<argn; i++){
                                if(strcmp(args[i],">")==0){
                                        int newstdout = open(args[i+1],O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
                                        close(1);
                                        dup(newstdout);
                                        close(newstdout);
                                        args[i]=NULL;
                                }
                                else if(strcmp(args[i],"<")==0){
                                        int newstdin = open(args[i+1],O_RDONLY);
                                        close(0);
                                        dup(newstdin);
                                        close(newstdin);
                                        args[i]=NULL;
                                }
                        }
                        execvp(cmd,args);
                        perror(cmd); //only happens when there is an error
                        endSh=1;
                }
                else{
                        if(!bg){ //if not a background process, wait for it
                                if (waitpid(childID,0,0)<0){//parent process
                                        endSh=1;
                                        perror("Error when waiting for child");
                                }
                        }
                }
          }
     }
	endSh=1;
}
