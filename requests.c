#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, const char *url, char *url_params, char* cookie, const char* auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    

    //PAS 1: Scriem numele metodei, calea, parametri din url (daca exista) si tipul protocolului
    if (url_params != NULL){

        sprintf(line, "GET %s?%s HTTP/1.1", url, url_params);
    }else{

        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    
    //PAS 2: Adaugam host-ul
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Host: %s", host);
    compute_message(message,line);

    //PAS 3 (optional): Adaugam headere si/ sau cookies, respectand forma protocolului

    //COOKIES
    if( cookie==NULL){
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Cookie: isLogged=true");
    compute_message(message,line);}
    else{
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "%s",cookie);
    compute_message(message,line);
    }
    

    //AUTHORIZATION LINE
    if( auth ){
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Authorization: Basic %s",auth);
    compute_message(message,line);
    }
    
   
    //PAS 4: Adaugam linia de final
    strcat(message, "\r\n");
    
    
    free(line);
    return message;
}


char *compute_post_request(char *host, const char *url,const char* type, char *form_data, const char* cookies,const char* auth)
{

    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    

    //PAS 1: Scriem numele metodei, calea si tipul protocolului
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    memset(line, 0, LINELEN*sizeof(char) );
    
        
    //PAS 2: Adaugam host-ul
    sprintf(line, "Host: %s", host);
    compute_message(message,line);
    memset(line, 0, LINELEN*sizeof(char) );

    
    //PAS 3: Adaugam headere si/ sau cookies, respectand forma protocolului

    //AUTHORIZATION    
    if( auth ){
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Authorization: Basic %s",auth);
    compute_message(message,line);
    }

    //COOKIES
    if ( cookies ){ 
    	sprintf(line, "%s",cookies);
    	compute_message(message,line);	}

    //CONTENT TYPE, CONTENT LENGTH
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Content-Type: %s",type);
    compute_message(message,line);
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "Content-Length: %ld",strlen(form_data) );
    compute_message(message,line);
    memset(line, 0, LINELEN*sizeof(char) );

    
    //PAS 4: Adaugam linia de final de antent
    strcat(message, "\r\n");
    

    //PAS 5 : Adaugam data
    memset(line, 0, LINELEN*sizeof(char) );
    sprintf(line, "%s",form_data);
    compute_message(message, line);

    free(line);    
    return message;
}
