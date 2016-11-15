//First name: Rachel
//Last Name: Mendiola
//cs login: rmend016
//SID : 861076777
//
//Lec sec: 001
//lab sec: 022
//TA: Mike Izbiki
//
//Description: Program to perform ls. 
//Purpose: To learn about directory pointers, dirents, and stat. 
//	   Using these three to make a program to perform an operation similar to 'ls -lgR'
//
//I hereby certify that the code in this file is ENTIRELY my own original work!
//

#include <dirent.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
using namespace std;

//this function will print the octal mode of a given st_mode vector
//@param buf is the stat buffer that contains the stat information we are reading
//@param t is the number of tabs
//postcondition: the octal mode of buf is printed to the console
void print(struct stat buf, int t){
        for(int i=0;i<t;++i) cout << '\t'; 
	S_ISDIR(buf.st_mode) ? cout << "d" : cout<<"-";
	(buf.st_mode & S_IRUSR) ? cout <<"r" : cout<<"-";
	(buf.st_mode & S_IWUSR) ? cout<<"w" : cout<<"-";
	(buf.st_mode & S_IXUSR) ? cout<<"x" : cout<<"-";
	(buf.st_mode & S_IRGRP) ? cout<<"r" : cout<<"-";
	(buf.st_mode & S_IWGRP) ? cout<<"w" : cout<<"-";
	(buf.st_mode & S_IXGRP) ? cout<<"x" : cout<<"-";
	(buf.st_mode & S_IROTH) ? cout<<"r" : cout<<"-";
	(buf.st_mode & S_IWOTH) ? cout<<"w" : cout<<"-";
	(buf.st_mode & S_IXOTH) ? cout<<"x" : cout<<"-"; 
}

//this function will display all directories and files in a given path
//@param startDir is the starting directory
//@param int t is the starting number of tabs (needed for recursion)
void my_ls(char * startDir, int t){
    struct stat buf;
    stat(startDir,&buf);
    if(!S_ISDIR(buf.st_mode)&& S_ISREG(buf.st_mode)){
	print(buf,t);
	cout << '\t' << buf.st_nlink << '\t' << buf.st_size << '\t'; 
	char *r = ctime(&buf.st_mtime);		//doctoring the time since epoch to make it look 
	r[16]='\0';				//pretty
	char *nt =r+4;
	cout << nt << '\t' << startDir << endl;
	return;
    }
    else if(!S_ISDIR(buf.st_mode)&&!S_ISREG(buf.st_mode)){
	cout << "Not a valid file or path" << endl;
	exit(1);
    }
    DIR * dirp;					//DIR pointer 
    char path[400]={0};				//cstring to hold the current path
    strcat(path, startDir);			//appends the starting directory into the path cstring
    if( !(dirp = opendir(startDir))){	
	cerr<< "error";				//if i can't open the directory it's an error and i exit
	exit(1);
    }
    dirent * currFile;
    while((currFile = readdir(dirp))){
	if(currFile->d_name[0]!='.'){
		char temp[400]={0};		//appends the new file to the original path
		strcat(temp,path);
		strcat(temp, "/");		
		strcat(temp, currFile->d_name);	
		stat(temp,&buf);			//gets stats on this path and store in buf
		if(S_ISDIR(buf.st_mode)){
		    cout << endl;			//space in between directories
		    for(int i=0;i<t;++i)cout<<'\t';	//adds correct tabing
		    cout << currFile->d_name<<endl;	//prints a directory name 
		    my_ls(temp,t+1);}
		if(!S_ISDIR(buf.st_mode)){		//if it's a file it prints all the info in here
		print(buf,t);				//print function to handle the octal mode
		cout << '\t' << buf.st_nlink << '\t' << buf.st_size << '\t';
		char *r = ctime(&buf.st_mtime);		//doctoring the time since epoch to make it look 
		r[16]='\0';				//pretty
		char *nt =r+4;
		cout<<nt<< '\t' << currFile->d_name << endl;
		}
   	 }
    }
    closedir(dirp);		//closes the directory we opened
}

int main(int argc, char *argv[]){
    if(argc==1)			//handles no arguments
	my_ls(".",0);
    else
	for(int i=1;i<argc;++i)		//handles multiple arguments
	    my_ls(argv[i],0);
    return 0;
}
