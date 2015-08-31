#include "gml_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[]) {
  
    struct GML_pair* list;
    struct GML_stat* stat=(struct GML_stat*)malloc(sizeof(struct GML_stat));
    stat->key_list = NULL;
    
    if (argc != 2)
	{
	    printf ("Usage: %s <gml_file> \n", argv[0]);
	    exit(1);
	}
    FILE* file = fopen (argv[1], "r");
    if (file == 0)
	{
	    printf ("\n No such file: %s\n", argv[1]);
	    exit(2);
	}
    GML_init ();

    list = GML_parser (file, stat, 0);
    if (stat->err.err_num != GML_OK)
	{
	    printf ("An error occured while reading line %d column %d of %s:\n", stat->err.line, stat->err.column, argv[1]);
	    switch (stat->err.err_num) {
	    case GML_UNEXPECTED:
		printf ("UNEXPECTED CHARACTER");
		break;
	    case GML_SYNTAX:
		printf ("SYNTAX ERROR"); 
		break;
	    case GML_PREMATURE_EOF:
		printf ("PREMATURE EOF IN STRING");
		break;
	    case GML_TOO_MANY_DIGITS:
		printf ("NUMBER WITH TOO MANY DIGITS");
		break;
	    case GML_OPEN_BRACKET:
		printf ("OPEN BRACKETS LEFT AT EOF");
		break;
	    case GML_TOO_MANY_BRACKETS:
		printf ("TOO MANY CLOSING BRACKETS");
		break;
	    default:
		break;
	    }
	    exit(6);
	}
		
    int node_count = 0;
    int* node_ids = NULL;
    int** matrix = NULL;

    struct GML_pair *tmp = list,
	*elements = NULL;
    int i;
    while (tmp) {
	if(strcmp(tmp->key, "graph") == 0 && tmp->kind == GML_LIST)
	    {
		elements = tmp->value.list;
		while(elements != NULL)
		    {
			if(strcmp(elements->key, "node") == 0 && elements->kind == GML_LIST)
			    node_count++;
			elements = elements->next;
		    }
		/* generate matrix */
		node_ids = (int*)malloc(sizeof(int)*node_count);
		matrix = (int**)malloc(sizeof(int*)*node_count);
		for(i = 0; i < node_count; i++)
		    {
			matrix[i] = (int*)malloc(sizeof(int)*node_count);
			memset(matrix[i], 0, sizeof(int)*node_count);
		    }
		memset(node_ids, 0, sizeof(int)*node_count);
		int i = 0;
		/* read nodes */
		elements = tmp->value.list;
		while(elements != NULL)
		    {
			if(strcmp(elements->key, "node") == 0 && elements->kind == GML_LIST)
			    {
				struct GML_pair *prop = elements->value.list;
				int id = -1;
				while(prop != NULL)
				    {
					if(strcmp(prop->key, "id") == 0 && prop->kind == GML_INT)
					    {
						id = prop->value.integer;
						break;
					    }
					prop = prop->next;
				    }
				if(id == -1)
				    {
					fprintf(stderr, "Cannot find id properties for node, please check the format of your GML file %s!", argv[1]);
					exit(3);
				    }
				node_ids[i++] = id;
			    }
			elements = elements->next;
		    }
		/* read edges */
		int edge_count = 0;
		elements = tmp->value.list;
		while(elements != NULL)
		    {
			if(strcmp(elements->key, "edge") == 0 && elements->kind == GML_LIST)
			    {
				struct GML_pair* prop = elements->value.list;
				int src = -1, dst = -1;
				while(prop != NULL)
				    {
					if(strcmp(prop->key, "source") == 0 && prop->kind == GML_INT)
					    src = prop->value.integer;
					else if(strcmp(prop->key, "target") == 0 && prop->kind == GML_INT)
					    dst = prop->value.integer;
					prop = prop->next;
				    }
				if(src == -1 || dst == -1)
				    {
					fprintf(stderr, "Cannot find source or target properties for edge, please check you GML file %s!\n", argv[1]);
					exit(4);
				    }
				int index1 = -1, index2 = -1;
				for(i = 0; i < node_count; i++)
				    {
					if(node_ids[i] == src)
					    index1 = i;
					if(node_ids[i] == dst)
					    index2 = i;
				    }
				if(index1 == -1 || index2 == -1)
				    {
					fprintf(stderr, "Cannot build edge from %d-%d. Cannot find related nodes. Please check your GML file %s\n", src, dst, argv[1]);
					exit(5);
				    }
				matrix[index1][index2] = 1;
				edge_count++;
			    }
			elements = elements->next;
		    }
		int j;
		printf("%d %d\n", node_count, edge_count);
		for(i = 0; i < node_count; i++)
		    {
			for(j = 0; j < node_count; j++)
			    printf("%d ", matrix[i][j]);
			printf("\n");
		    }
		printf("\n");
	    }
	tmp = tmp->next;
    }
    GML_free_list (list, stat->key_list);
    return 0;
}
