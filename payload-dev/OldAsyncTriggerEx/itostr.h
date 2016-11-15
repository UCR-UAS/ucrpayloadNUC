#include <iostream>
#include <string>

/* Takes an intiger and returnes a string 
 * of length 6 consisting of prepended 
 * zeros to the given intiger value */
std::string prep_zeros6(int i){
	std::string s="000000";
	int c=0;
	for(int j=i;j!=0;j=j/10)
		s[5-c++]=(j%10)+'0';
	return s;
}

/* Takes an intiger and returnes a string 
 * of length 'size' consisting of prepended 
 * zeros to the given intiger value 'i' */
std::string prep_zeros(int i,int size=6){
	char *c=new char[size];
	for(int j=size-1;j>=0;j--,i/=10){
		c[j]=(i%10)+'0';
	}
	std::string s(c);	
	return s;
}
