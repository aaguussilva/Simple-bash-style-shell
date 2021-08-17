#include "command.h"
#include "strextra.h"
#include <gmodule.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

struct scommand_s {
	GQueue * str; 
	char * in;		
	char * out;  	
};

scommand scommand_new(void){  						// Crea un nuevo comando
	scommand result = calloc(1, sizeof(struct scommand_s));		//Crea espacio en memoria
	assert(result != NULL);												
	result->str = g_queue_new();								// Crea GQueue
	result->in = NULL;
	result->out = NULL;
	assert(result != NULL && scommand_is_empty(result) && scommand_get_redir_in(result) == NULL &&
 	scommand_get_redir_out(result) == NULL);
	return result;
}

scommand scommand_destroy(scommand self){	 // destruye un comando 
	assert(self != NULL);
	g_queue_free_full(self->str, g_free);	// libera cada una de las cadenas
	if(self->in != NULL){					
		g_free(self->in);					// libero entrada
		self->in = NULL;
	}
	if(self->out != NULL){
		g_free(self->out);					//libero salida
		self->out = NULL;
	}
	free(self);								//libero la estructura 
	self = NULL;
	assert(self == NULL);
	return (self);
}

void scommand_push_back(scommand self, char * argument){	//Agrega un cmd por atras de la secuencia
	assert(self != NULL && argument != NULL);
	g_queue_push_tail(self->str,argument);
}

void scommand_pop_front(scommand self){						//Quita el primer comando
	assert(self!=NULL && !scommand_is_empty(self));
    gpointer head = g_queue_pop_head(self->str);			//Elimina el primer elemento de GQueue , que es el primer comando		
    g_free(head);
}

void scommand_set_redir_in(scommand self, char * filename){ // Define la redireccion de entrada
	assert(self != NULL);
	g_free(self->in);
	self->in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){	// Define la redireccion de salida
	assert(self != NULL);
	g_free(self->out);
	self->out = filename;
}

bool scommand_is_empty(const scommand self){			// Nos dice si un comando es vacio
	assert(self != NULL);
	return (g_queue_is_empty(self->str));
}

unsigned int scommand_length(const scommand self){		//Length de un comando
	assert(self != NULL);
	guint n = g_queue_get_length(self->str);	
	assert((n==0) == scommand_is_empty(self));
	return n;
}

char * scommand_front(const scommand self){				// toma y devuelve la cadena del frente
	assert(self != NULL && !scommand_is_empty(self));
	char * result = g_queue_peek_head(self->str);		//peek_head devuelve el primer elemento
	assert(result != NULL);
	return result;
}

char * scommand_get_redir_in(const scommand self){		// retorna la redireccion de entrada
	assert(self != NULL);
	return (self->in);
}

char * scommand_get_redir_out(const scommand self){ 	// retorna la redireccion de salida
	assert(self != NULL);
	return (self->out);
}

char * scommand_to_string(const scommand self){	//Representacion de un comando simple a string
	assert(self!=NULL);
	guint len_list = g_queue_get_length(self->str);		//length del comando
	gchar * string = strdup("");						//Asignamos memoria a nuestra cadena de string
	for(unsigned int i = 0 ; i < len_list; i++){		//Usando strmerge vamos armando la cadena de string
		string = strmerge(string, g_queue_peek_nth(self->str,i));
		string = strmerge(string, " "); 
	}
	if(scommand_get_redir_in(self) != NULL){			//Agregamos la redireccion de entrada a la cadena
		string = strmerge(string, "<"); 
		string = strmerge(string, " ");
		string = strmerge(string, self->in);
		string = strmerge(string, " ");
	}
	if(scommand_get_redir_out(self) != NULL){			//Agregamos la redireccioon de salida a la cadena
		string = strmerge(string, ">");
		string = strmerge(string, " ");
		string = strmerge(string, self->out);
	}
	assert(scommand_is_empty(self) || scommand_get_redir_in(self)==NULL || scommand_get_redir_out(self)==NULL || strlen(string)>0);
	return string;
}

/*---------------------------Pipeline---------------------------------------*/

struct pipeline_s {
	GQueue * sc_pipe;
	bool wait;
};

pipeline pipeline_new(void){														// creo un nuevo pipeline
	pipeline pipe = calloc(1,sizeof(struct pipeline_s));							// creo espacio en la memoria
	pipe->sc_pipe = g_queue_new();													// creo Gqueue
	pipe->wait = true;																// por defecto, espera
	assert(pipe != NULL && pipeline_is_empty(pipe) && pipeline_get_wait(pipe));
	return pipe;
}

static void destroy_scmd(gpointer scmd){		// LLama a la funcion scommand_destroy(), para coincidir con el tipo de g_qeue_free_full()
	scommand_destroy(scmd);
}

pipeline pipeline_destroy(pipeline self){		//Destruye un pipeline
	assert(self != NULL);
	g_queue_free_full(self->sc_pipe, &destroy_scmd);	//Esta funcion libera completamente la memoria del queue
	free(self);
	self = NULL;
	return self;
}

void pipeline_push_back(pipeline self, scommand sc){	//Agrega un comando por detras a la secuencia
	assert(self != NULL && sc != NULL);
	g_queue_push_tail(self->sc_pipe,sc);
	assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){					//Quita el primer comando de la secuencia
	assert(self != NULL && !pipeline_is_empty(self));
    gpointer head = g_queue_pop_head(self->sc_pipe);
    head = scommand_destroy(head);
    head = NULL;
}

void pipeline_set_wait(pipeline self, const bool w){  // Define si el pipeline tiene que esperar o no
	assert(self != NULL);
	self->wait = w;
}

bool pipeline_is_empty(const pipeline self){ 		//Indica si la secuencia de comandos simples tiene longitud 0
	assert(self != NULL);
	bool b;
	return (b = g_queue_is_empty(self->sc_pipe));
}

unsigned int pipeline_length(const pipeline self){	//Da la longitud de la secuencia de comandos simples
	assert(self != NULL);
	guint n = g_queue_get_length(self->sc_pipe); 	// esta funcion retorna, la longitud
	assert((n==0) == pipeline_is_empty(self));
	return n;
}

scommand pipeline_front(const pipeline self){		//Devuelve el comando simple de adelante de la secuencia
	assert(self!=NULL && !pipeline_is_empty(self));
	scommand sc = g_queue_peek_head(self->sc_pipe); //peek_head devuelve el primer elemento
	assert(sc != NULL);
	return sc;
}

bool pipeline_get_wait(const pipeline self){		//Consulta si el pipeline tiene que esperar o no
	assert(self!=NULL);
	return (self->wait);
}

char * pipeline_to_string(const pipeline self){		// Representación del pipeline en una cadena
	assert(self!=NULL);
	gchar * merge = strdup("");						// strdup duplica el puntero a char asignandole memoria dinamica
	gchar * cmd = NULL;								
	guint len = pipeline_length(self);
	for (guint i = 0 ; i < len ; i++){				// Voy ingresando representacion de comandos simples
		cmd = scommand_to_string(g_queue_peek_nth(self->sc_pipe,i)); // Uso la funcion scommand_to_string, para en cada ciclo convertir el comando simple en un string
		merge = strmerge(merge,cmd);				// Concateno los commandos simples reasignando la memoria necesitada. (ver strextra.c)
		g_free(cmd);								// Libero memoria, (se creo memoria al usar scoomand_to_string)
		if(i != (len-1)){							// Agrego pipe en cada ciclo si no es el ultimo comando
			merge = strmerge(merge," | ");
		}										
		if(i == len-1 && !pipeline_get_wait(self)){ // Agrego & al final del ultimo commando simple si el pipeline no espera
			merge = strmerge(merge," &");
		}
	}
	assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(merge)>0);
	return merge;
}
