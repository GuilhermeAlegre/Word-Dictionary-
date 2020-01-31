#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>




/*...........................Read Words..........................*/

typedef struct file_data 
{
	long word_pos; /* Word position */
	long word_num; /* Word number */
	char word[64];
	FILE *fp; 
	long current_pos; /* Current position */
	
}
file_data_t; 


int open_text_file(char *file_name, file_data_t *fd)
{
	fd->fp = fopen (file_name, "rb");
	if(fd->fp == NULL)
		return -1;
	fd->word_pos=-1;
	fd->word_num=-1;
	fd->word[0]='\0';
	fd->current_pos = -1;
	return 0;
}

void close_text_file(file_data_t *fd)
{
	fclose(fd->fp);
	fd->fp = NULL;
}

int read_word(file_data_t *fd)
{
	int i,c;

	do 
	{
		c=fgetc(fd->fp);
		if(c == EOF)
			return -1;
		fd->current_pos++;
	}
	while(c <= 32);

	// record word
	fd->word_pos = fd->current_pos;
	fd->word_num++;
	fd->word[0] = (char)c;

	for(i = 1;i < (int)sizeof(fd->word) - 1;i++)
	{
		c = fgetc(fd->fp);
		if(c == EOF)
			break; // end file	
		fd->current_pos++;
		if(c <= 32)
			break; // terminate word
		fd->word[i] = (char)c;
	}
	fd->word[i] ='\0';
	
	return 0;

}

/*..............................Hash_Function............................*/

unsigned int hash_function(const char *str,unsigned int s)
{
	static unsigned int table[256];
	unsigned int crc,i,j;

	if(table[1] == 0u) // do we need to initialize the table[] array?
		for(i = 0u;i < 256u;i++)
			for(table[i] = i,j = 0u;j < 8u;j++)
				if(table[i] & 1u)
					table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
				else
					table[i] >>= 1;
			crc = 0xAED02019u; // initial value (chosen arbitrarily)
			while(*str !='\0')
				crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
			return crc % s;
}
/*..............................Hash_Table...............................*/

/* Node Struct*/ 

typedef struct node 
{
	char Word[64];
	long frist; /* Word FristPos temp_data->smallest=(word_pos - temp_data->last); */ 
	long last; /* WordLastPos */
	long smallest; /* smallest distance between words */  
	long largest; /* largest distance between words */ 
	long totalDistance; /* Total distance to calculate average */
	int countWord; /* NumberOfword */  
	struct node *next; 
}node;

/* Hash table struct*/

typedef struct hashTable
{
	/* Capacity of the array used to index the hash table */
	unsigned long size; // possivelmente será melhor alterar para unsigned long 
	/* Current number of elements stored in the hash table */
	unsigned long load;
	/* array of the pointer to node */
	node **table; 
}hashTable;

/*Load factor */
#define loadFactor 0.8

void insert_node(const char *key, node *newNode, hashTable *hash_table);

/*...................Create a new Hash table...............................*/ 

hashTable *hd_create(unsigned long size)
{
	hashTable *hash_table = NULL; 

	/* check the value of the size */
	if (size < 1 ) return NULL; 

	/* Allocate the table itself*/
	if ((hash_table = malloc(sizeof(hashTable))) == NULL)
	{
		printf("O erro está aqui");
		return NULL; 
	}
	/* Allocate de pointers to the head nodes */
	if ((hash_table->table = malloc(sizeof(node*) * size ))== NULL)
		return NULL; 

	/*Inicialization of the hash table whith NULL */
	for (int i = 0 ; i < size ; i++)
		hash_table->table[i] = NULL;
	
	/* save the size of the hash table in the hash_table struct */
	hash_table->size = size; 
	hash_table->load=0; 
	 
	return hash_table;
}
/*.....................Free Hash table.........................................*/

void hd_free(hashTable* hash_table)
{
	/*Inicialization variables for free Linked list*/

	node *node_list, *temp_nodeList;
	/*Check if there is hash table */
	if(hash_table == NULL) return; 
	/*Free the memory for every item in hash table */
	for (int i = 0 ; i < hash_table->size; i++)
	{
		node_list =hash_table->table[i];
		while ( node_list != NULL)
		{
			temp_nodeList = node_list;
			node_list=node_list->next;
			// Não se é preciso limpar os outros dados da estrutura 
			free(temp_nodeList->Word);
			free(temp_nodeList);
		}

		/*Free the table itsefl*/
		free(hash_table->table);
		free(hash_table);
		
	}
} 
/*.......................Resize hash table..................................*/

void hd_resize(hashTable *hash_table, int growth_rate)
{
	hashTable *hd_new;
	node *link_list,*next;
	node *temp; // ???
	hd_new = hd_create(hash_table->size * growth_rate);
	
	/* go all over the hash table and copy nodes to new hash table */
	for (int i = 0 ; i < hash_table->size ; i++)
	{
		link_list = hash_table->table[i];
		
		while (link_list != NULL)
		{
			next = link_list->next;
			insert_node(link_list->Word, link_list, hd_new);
			//  Será necessario criar uma variable temp para guarda o no e fazer free ??
			link_list=next;
		}
	}
	/* free the memory of the old hash table */
	
	//não sei bem como se liberta a memoria, se podemos libertar logo a estrutura 
	//ou se temos que eliminar cada elemento da struct e por a fim a struct 
	free(hash_table->table);
	*hash_table = *hd_new;
	free(hd_new);
}

/*.......................Find key in hash table............................*/

node *find(hashTable *hash_table, const char *key)
{	
	/* yet to find node */
	node *temp_node;

	unsigned int idx = hash_function(key,hash_table->size);
	
	temp_node= hash_table->table[idx]; 

	while (temp_node != NULL &&strcmp(temp_node->Word,key)!=0)
		temp_node = temp_node->next;
		
	return temp_node;
}
/*...........................Stats Hash table...............................*/

void Stats_wors(node *temp_data, long word_pos,char *word )
{	
	//distancia entre o final da primeira palavra e o inicio da segunda palavra
	long distWord = (word_pos - temp_data->last - strlen(word));
	temp_data->totalDistance += distWord;
	if (distWord < temp_data->smallest) 
		temp_data->smallest = distWord;
	if (distWord > temp_data->largest)
		temp_data->largest = distWord; 
	/*neste momento de so aparece uma vez uma palavra a distancia minima será m= 922337....... ,
	E a distancia maxima é M=-1*/   
	temp_data->countWord += 1; 
	temp_data->last = word_pos; 	
}

/*...........................Create a new node..............................*/

node *create(const char *key, long word_pos)
{	
	/* create the item for insert in linked list */
	node *newNode;
	newNode = (node *)malloc(sizeof(node));
	strcpy(newNode->Word,key);
	newNode->countWord = 1; 
	newNode->last = word_pos;
	newNode->frist= word_pos;  
	newNode->smallest = LONG_MAX; 
	newNode->largest = -1; 
	newNode->next = NULL;

	return newNode;
}
/*..........................Add a new the hash table...........................*/

void insert_node(const char *key, node *newNode, hashTable *hash_table)
{
	/* Add the node to hash table */
	unsigned int idx = hash_function(key,hash_table->size);
	node *temp;
	if( hash_table->table[idx]== NULL){
		hash_table->table[idx]=newNode;
	}
	else {
		
		temp=hash_table->table[idx];
		while (temp->next != NULL)
			temp=temp->next;
		temp->next = newNode;	
	}
	newNode->next = NULL;
	hash_table->load += 1 ;
	
	 /*resize of hash table */
	if (hash_table->load > hash_table->size*loadFactor)
		hd_resize(hash_table, 2); 
}

/*..........................Print of the hash table............................*/

void print(hashTable *hash_table)
{
	for ( int i = 0 ; i <hash_table->size ; i++)
	{
		node *temp=hash_table->table[i];
		printf("hash_table[%d]-->", i);
		while (temp != NULL)
		{ 	
			float average; 
			average = (float)temp->totalDistance/(temp->countWord-1);
			
			
			/*In case onle one word appears the minimum, maximum and average are -1*/
			if ( temp->smallest == LONG_MAX )
			{
				temp->smallest = -1; 
				/* não seria necessário, uma vez, que é inicializado com -1 */
				temp->largest = -1;
				average = -1;
				printf("%s(F:%d; FP:%ld; LP:%ld; One Time)-->",temp->Word , temp->countWord, temp->frist , temp->last);		
            temp = temp->next;
			}
			else
			{	

			printf("%s(F:%d; FP:%ld; LP:%ld; m:%ld; M=%ld; Ave=%.2f)-->",temp->Word , temp->countWord, temp->frist , temp->last, temp->smallest, temp->largest ,average);		
            temp = temp->next;
			}
		}
		printf("NULL  \n" );	
	}
}
/*.................................MAIN........................................*/


int main(int argc, char const *argv[])
{

	file_data_t dic;
	int readWord;

	node *temp_data;
	node *newNode;

	hashTable *hash_table;
	int size_hash_table = 5; 
	
	/*open_text_file("SherlockHolmes.txt", &dic);*/
	open_text_file("teste.txt", &dic);
	hash_table = hd_create(size_hash_table);
	
	while(read_word(&dic)==0){
		temp_data = find(hash_table,dic.word);
		if ( temp_data == NULL ){
			newNode = create(dic.word ,dic.word_pos);
			insert_node(dic.word, newNode, hash_table);
		}
		else{ 
			Stats_wors(temp_data, dic.word_pos, dic.word);
		}
	}
	close_text_file(&dic);
	print(hash_table);
	hd_free(hash_table);

	return 0;
}
