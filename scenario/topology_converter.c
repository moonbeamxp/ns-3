#include <stdio.h>
#include <stdlib.h>

void main()
{
  int i=0,issrc=0;
  char node[5],src[5],des[5],c,last;
  FILE *fpin, *fpout;
  
  if((fpin=fopen("input.txt","r"))==NULL)
  {
    printf("Can not open input file.\n");
    exit(0);
  }
  
  if((fpout=fopen("output.txt","w"))==NULL)
  {
    printf("Can not open output file.\n");
    exit(0);
  }
  
  fprintf(fpout,"router\n");
  fprintf(fpout,"#node\tcomment\tyPos\txPos\n");
  
  while((c=fgetc(fpin))!='\n')
  {
    if(c>='0'&&c<='9')
    {
      node[i]=c;
      i++;
    }
    if(c==','||c==']')
    {
      node[i]='\0';
      i=0;
      fprintf(fpout,"%s\tNA\t0\t0\n",node);
    }
  }
  
  fprintf(fpout,"link\n");
  fprintf(fpout,"#    srcNode dstNode   bandwidth     metric    deLay   queue\n");
  
  i=0;
  issrc=1;
  
  while((c=fgetc(fpin))!=EOF)
  {
    if(c==','&&last>='0'&&last<='9')
    {
      src[i]='\0';
      i=0;
      issrc=0;
    }
    if(c==')') 
    {
      des[i]='\0';
      i=0;
      issrc=1;
      fprintf(fpout,"\t%s\t%s\t100Mbps\t\t1\t10ms\t100\n",src,des);
    }
    if(c>='0'&&c<='9')
      if(issrc==1) src[i++]=c;
      else         des[i++]=c;
    
    last=c;
  }
  fclose(fpout);
  fclose(fpin);
}
