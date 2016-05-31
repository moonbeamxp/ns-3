#include <stdio.h>
#include <stdlib.h>

void main()
{
  char node[5];
  while(node[0]!='q')
  {
    scanf("%s",node);
    printf("Names::Find<Node> (\"%s\"),\t",node);
    scanf("%s",node);
    printf("Names::Find<Node> (\"%s\"),\t",node);
    scanf("%s",node);
    printf("Names::Find<Node> (\"%s\"),\n",node);
  }
}
