//Group 51: Atul Anand, Dashmesh Singh, Kanupriya Singh

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<ctype.h>
#include<signal.h>
#include<fcntl.h>

void shell(int);
void reset(void);
static void split(char *cmd);
static char* args[512];

int zone=0;
int fd[2];
pid_t pid;
int ret;
int fd_multiple=0;

void reset()
{
	zone=0;
	fd_multiple=0;
	fd[0]=0;
	fd[1]=0;
	args[0]=0;
}

static void removelastspace(char* s)
{
	char* tmp= strchr(s, '\0');
	tmp= tmp-1;
	if(strcmp(tmp," ")==0)
	{
		*tmp= '\0';
		removelastspace(s);
	}
}

void pipeSequence(char *args[], int pipesCount,int k) 
{
	ret=pipe(fd);
	if(ret==-1){
		printf("Unable to pipe\n");
		return;
	}
	pid=fork();
	if(pid==-1)
	{
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)					// Child Process
	{
		dup2(fd_multiple,0);
		if(zone<pipesCount){
			dup2(fd[1],1);
		}
		close(fd[0]);

		for(int j=0;j<k;j++)
		{
			if(strcmp(args[j],">") == 0)
			{
				args[j]=NULL;
				j=j+1; 		//filename

				close(1);

				char fn[100];
				strncpy(fn,args[j],99);

				int lfd=open(fn, O_RDWR | O_TRUNC);
			
				if(lfd==-1)
				{
					open(fn, O_CREAT | O_RDWR | O_TRUNC);
				}
				args[j]=NULL;
			}
			else if(strcmp(args[j],">>") == 0)
			{
				args[j]=NULL;
				j=j+1; 			//filename
				
				close(1);

				char fn[100];
				strncpy(fn,args[j],99);

				int lfd=open(fn, O_RDWR | O_APPEND);

				if(lfd==-1)
				{
					open(fn, O_CREAT | O_RDWR | O_APPEND);
				}
				args[j]=NULL;
			}
			else if(strcmp(args[j],"2>&1") == 0)
			{
				close(2);
				dup(1);
				args[j]=NULL;
			}
			else if(args[j][0]=='2')
			{
				char* localptr= &args[j][0];
				localptr=localptr+2;
				char fn[100];
				strncpy(fn,localptr,99);
									
				close(2);
				close(1);

				int lfd=open(fn, O_RDWR | O_TRUNC);

				if(lfd==-1)
				{
					open(fn, O_CREAT | O_RDWR | O_TRUNC);
				}

				//Move the open statement before close (1) to see effect of stderr on output file. 
				//Here we move our own error message to see effect of error

				args[j]=NULL;
			}
			else if(args[j][0]=='1')
			{
				char* localptr= &args[j][0];
				localptr=localptr+2;
				char fn[100];
				strncpy(fn,localptr,99);

				close(1);
				
				int lfd=open(fn, O_RDWR | O_TRUNC);

				if(lfd==-1)
				{
					open(fn, O_CREAT | O_RDWR | O_TRUNC);
				}
				args[j]=NULL;					
			}	
			else if(strcmp(args[j],"<") == 0)
			{
				args[j]=NULL;
				j=j+1;			//filename
				
				close(0);
				
				char fn[100];
				strncpy(fn,args[j],99);

				int lfd=open(fn, O_RDONLY);
				
				if(lfd==-1)
				{
					open(fn, O_CREAT | O_RDWR | O_TRUNC);
				}
				args[j]=NULL;
			}		
		}

		ret=execvp(args[0], args);
		if(ret==-1)
		{
			printf("Unable to execute\n");	
			exit(0);	
		}
	}
    else				// Parent Process
    {
		wait(NULL);
		close(fd[1]);
		fd_multiple=fd[0];
     
    }
}

int main(){
	printf("Welcome to basic shell\n");

	signal(SIGINT, shell);
	shell(0);
	return 0;
}

void shell(int signal)
{
		char command[1024];
		int flag=1;
		printf("Back to Shell\n");

		do
		{
			printf("$");

			scanf("%[^\n]%*c",command);

			removelastspace(command);

			int pipesCount=0;
			for(int i=0;i<strlen(command);i++){
				if(command[i]=='|'){
					pipesCount++;			
				}
			}
			//printf("Number of pipes : %d\n", pipesCount);

			char pipeCommands[pipesCount+1][100];
			int j=0,k=0;
			for(int i=0;i<strlen(command);i++){
				if(command[i]=='|'){
					pipeCommands[j][k-1]='\0';
					j++;
					k=0;									
				}
				else if(command[i]==' '){
					pipeCommands[j][k]='$';
					k++;			
				}
				else{
					pipeCommands[j][k]=command[i];
					k++;
	
				}			
			}
			pipeCommands[j][k]='\0';

			for(int i=0; i<pipesCount+1;i++)			// Loop for each pipe sequence
			{
				//printf("Pipe command : %s\n", pipeCommands[i]);
				char *args[10];	
				int k=0;
				if(i==0){								// First command doesn't has starting symbol $
					args[0]=&pipeCommands[i][0];
					k++;
				}

				unsigned int size=strlen(pipeCommands[i]);
				for(int j=0;j<size;j++)
				{
					if(pipeCommands[i][j]=='$'){
							pipeCommands[i][j]='\0';
							args[k]=&pipeCommands[i][j+1];
							k++;			
					}
				}
				args[k]=NULL;

				if(strcmp(args[0], "exit")==0)
				{
					exit(0);
				}

				pipeSequence(args, pipesCount,k);
				zone++;
			}

			reset();	

	}while(flag==1);
}