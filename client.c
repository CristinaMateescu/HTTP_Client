#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// Get Ip by host_name 
void get_ip(char* name, char* ip)
{
	int ret;
	struct addrinfo hints, *result, *p;

	// Set hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;


	// Get addresses
	ret = getaddrinfo(name, NULL, &hints, &result);
	if(ret != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		return;
	}


	// Iterate through addresses
        char adresa_ip[100];
	void *ptr;
	p = (struct addrinfo*)result;
	while(p) {
		switch (p->ai_family) {
		case AF_INET:
			ptr =  &((struct sockaddr_in *) p->ai_addr)->sin_addr;
			break;

		case AF_INET6:
			ptr =  &((struct sockaddr_in *) p->ai_addr)->sin_addr;
			break;
		}

		
		inet_ntop(p->ai_family, ptr, adresa_ip, sizeof(adresa_ip));
		p = p->ai_next;
	}

	// Free allocated data
	freeaddrinfo(result);
       
        //Return last ip in list
        memcpy(ip, adresa_ip,strlen(adresa_ip)+1);
}


//Main
int main()
{
   
    char *message;
    char *response;
    int sockfd;


    /* ------------------- TASK1 ------------------- */


    char* host_ip="185.118.200.35";
    char* url="/task1/start";
    char* url_params=NULL;
    int portno = 8081  ,ip_type=AF_INET ,socket_type=SOCK_STREAM ;

    /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(host_ip, portno, ip_type, socket_type, 0);

    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    message = compute_get_request(host_ip, url, url_params,NULL, NULL);

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);

    //printf("%s\n",response);

    /* ------------------- TASK2 ------------------- */	


    /* ----------- Parsare COOKIES si ENUNT ---------*/


    /* Numara cookie-uri */
    int cookie_count = 0;
    const char* aux = response;
    while ( aux = strstr(aux, "Set-Cookie")){
	cookie_count++;
	aux++;}
   

    /* Aloca dinamic memorie pentru a stoca liniile ce contin cookie-uri */
    char** Cookies = NULL;
    if ( cookie_count ) 
	Cookies = (char**)malloc(sizeof(char*)*cookie_count);
    

    /* Parsez liniile */
    char* token = NULL;
    token = strtok(response, "\n");
    int cookie_index = 0;
    int cookie_length = 0;
    while(token ) {
	
	/* Parcurg liniile pana la prima linie goala */
	if(strlen(token)==1){
		token = strtok(NULL, "\n");
		break;}

	/* Verific daca linia curenta reprezinta linia "Set-cookie" pentru a extrage cookie-urile */
	if ( strstr(token, "Set-Cookie" ) ){
		
		Cookies[cookie_index] = (char*)malloc(sizeof(char)*strlen(token));
		memcpy(Cookies[cookie_index],token+12, strlen(token));
		cookie_index++;
	 	cookie_length+=strlen(token);}
	token = strtok(NULL, "\n");
     }


	
     /* Concatenam cookie-urile */
     char * cookie = (char*)malloc((cookie_length+7)*sizeof(char));
     sprintf(cookie, "Cookie: ");
     char* index = cookie+8;
     int i;
     for( i=0; i<cookie_count; i++ ){
	
	char* cookie_token = strtok(Cookies[i], ";");
	if ( i<cookie_count-1){
		sprintf(index, "%s;", cookie_token );
		index+=1+strlen(cookie_token);}
	else {
		sprintf(index, "%s", cookie_token );
		index+=strlen(cookie_token);}
     }


     /* Eliberam memoria cookie-urilor */
     for ( i=0; i<cookie_count; i++ )
	free(Cookies[i]);
     free(Cookies);


   
    /* --------------- PARSARE ENUNT --------------------*/

    JSON_Value  *enunt = json_parse_string(token);
    JSON_Object *obj = json_value_get_object(enunt);


    /* ---------------- EXTRAGERE CAMPURI --------------- */

    /* Url, method, type ( daca e cazul) */
    const char* url_task2 =  json_object_get_string (obj, "url");
    const char* metoda_task2 =  json_object_get_string (obj, "method");

    const char* type_task2 = NULL;
    if ( !strcmp(metoda_task2, "POST" ))
	type_task2 =  json_object_get_string (obj, "type");


   /* Extragere din campul data - daca e cazul */
   JSON_Object* data_task2 = NULL;
   data_task2 = json_object_get_object (obj, "data");
   
   size_t count = 0;
   char ** values = NULL; 
   int values_size = 0;
   
   /* Parsare campuri data - stocare */
   if ( data_task2 ) {

   	count = json_object_get_count (data_task2);
	values = (char**)malloc(sizeof(char*)*count);	

	int i;
   	for ( i=0; i<count; i++ ){

		const char* key = json_object_get_name(data_task2, i);
		const char* val = json_object_get_string(data_task2, key);
		values[i] = (char*)malloc(sizeof(char)*(strlen(key)+strlen(val)+2));
		sprintf(values[i],"%s=%s",key, val);
   		values_size+=strlen(key)+strlen(val)+2;
   	}
   }
   
     
    /* Concatenam campurile din data */
    char* data = (char*)malloc(sizeof(char)*(values_size+count-1) );
    index = data;
    for( i=0; i<count; i++ ){
	
	if(i<count-1){
	sprintf(index, "%s&", values[i] );
	index+=strlen(values[i])+1;}
	else{
	sprintf(index, "%s", values[i] );
	index+=strlen(values[i]);
	}
        
	
     }

    /* Eliberam memoria */
    for( i=0; i<count; i++ )
	free(values[i]);
    free(values);

  
    /* ---------------- TRIMITERE CERERE ---------------- */
    
    /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(host_ip, portno, ip_type, socket_type, 0);

    
    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    if ( !strcmp(metoda_task2, "POST" )) 
    	message = compute_post_request(host_ip, url_task2,type_task2, data, cookie,NULL);

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    free(response);
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);

    
    /* Eliberare memorie */
    free(cookie);
    free(data);
    json_value_free (enunt);


     
     /* ------------------- TASK3 ------------------- */

     /* ----------- Parsare COOKIES si ENUNT ---------*/
 
    /* Numara cookie-uri */
    cookie_count = 0;
    aux = response;
    while ( aux = strstr(aux, "Set-Cookie")){
	cookie_count++;
	aux++;
    }
   
    /* Aloca dinamic memorie pentru a stoca liniile ce contin cookie-uri */
    Cookies = NULL;
    if ( cookie_count ) 
	Cookies = (char**)malloc(sizeof(char*)*cookie_count);
    


    /* Parsez liniile */
    token = NULL;
    token = strtok(response, "\n");
    cookie_index = 0;
    cookie_length = 0;
    while(token ) {
	
	/* Parcurg liniile pana la prima linie goala */
	if(strlen(token)==1){
		token = strtok(NULL, "\n");
		break;}


	/* Verific daca linia curenta reprezinta linia "Set-cookie" pentru a extrage cookie-urile */
	if ( strstr(token, "Set-Cookie" ) ){
		
		Cookies[cookie_index] = (char*)malloc(sizeof(char)*strlen(token));
		memcpy(Cookies[cookie_index],token+12, strlen(token));
		cookie_index++;
	 	cookie_length+=strlen(token);
			
		}

	token = strtok(NULL, "\n");


     }

	
     /* Concatenam cookie-urile */
     cookie = (char*)malloc((cookie_length+7)*sizeof(char));
     sprintf(cookie, "Cookie: ");
     index = cookie+7;
     for( i=0; i<cookie_count; i++ ){
	
	char* cookie_token = strtok(Cookies[i], ";");
	if ( i<cookie_count-1){
	sprintf(index, "%s;", cookie_token );
	index+=1+strlen(cookie_token);}
	else {
	sprintf(index, "%s", cookie_token );
	index+=strlen(cookie_token);
	}
        
	
     }


     /* Eliberam memoria cookie-urilor */
     for ( i=0; i<cookie_count; i++ )
	free(Cookies[i]);
     free(Cookies);

    /* --------------- PARSARE ENUNT --------------------*/ 
    
    /* Parsare response */
    JSON_Value  *enunt_task3 = json_parse_string(token);
    JSON_Object *obj_task3 = json_value_get_object(enunt_task3);

    /* ---------------- EXTRAGERE CAMPURI --------------- */

    /* Extragere din campurile url, method, type - daca e cazul */
    const char* url_task3 =  json_object_get_string (obj_task3, "url");
    const char* metoda_task3 =  json_object_get_string (obj_task3, "method");

   /* Extragere din campul data - daca e cazul */
   JSON_Object* data_task3 = NULL;
   data_task3 = json_object_get_object (obj_task3, "data");

   count = 0; 
   char* param;
   const char* auth;
   if ( data_task3 ) {

   	count = json_object_get_count (data_task3);
        for ( i=0; i<count; i++ ){

		const char* key = json_object_get_name(data_task3, i);
		const char* val = json_object_get_string(data_task3, key);
		if ( val )
			auth=val;
		else {
   			
			JSON_Object *query_params = json_object_get_object (data_task3, key);
			int params_count = json_object_get_count (query_params);
			
			char ** params = (char**)malloc(sizeof(char*)*params_count);
   			int params_size = 0;
			int j;
			for ( j=0; j<params_count; j++ ){

				const char* param_key = json_object_get_name(query_params, j);
				const char* param_value = json_object_get_string(query_params, param_key);
				params[j] = (char*)malloc(sizeof(char)*(strlen(param_key)+strlen(param_value)+2));
				sprintf(params[j],"%s=%s",param_key, param_value);
   				params_size+=strlen(param_key)+strlen(param_value)+2;
				
			}

			/*Concatenare params*/
			param = (char*)malloc(sizeof(char)*(params_size+params_count-1) );
   			char* p = param;
    			for( j=0; j<params_count; j++ ){
	
				if(j<params_count-1){
				sprintf(p, "%s&", params[j] );
				p+=strlen(params[j])+1;}
				else{
				sprintf(p, "%s", params[j] );
				p+=strlen(params[j]);
				}
	
     			}
			
			/*Free memory*/
			for ( j=0; j<params_count; j++)
				free(params[j]);
			free(params);

		}	
	   }
	
     }
     /* ---------------- TRIMITERE CERERE ---------------- */

    /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(host_ip, portno, ip_type, socket_type, 0);

    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    if ( !strcmp(metoda_task3, "GET" )){
	url_params = malloc(sizeof(char)*(strlen(param)+31));
	sprintf(url_params,"raspuns1=omul&raspuns2=numele&%s",param); 
    	message = compute_get_request(host_ip, url_task3, url_params,cookie, auth);}

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    free(response);
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);

    /* Eliberare memorie */
    free(cookie);
    free(param);
    free(url_params);

    /* ------------------- TASK4 ------------------- */
     

    /* Numara cookie-uri */
    cookie_count = 0;
    aux = response;
    while ( aux = strstr(aux, "Set-Cookie")){
	cookie_count++;
	aux++;
    }
   
    /* Aloca dinamic memorie pentru a stoca liniile ce contin cookie-uri */
    Cookies = NULL;
    if ( cookie_count ) 
	Cookies = (char**)malloc(sizeof(char*)*cookie_count);
    


    /* Parsez liniile */
    token = NULL;
    token = strtok(response, "\n");
    cookie_index = 0;
    cookie_length = 0;
    while(token ) {
	
	/* Parcurg liniile pana la prima linie goala */
	if(strlen(token)==1){
		token = strtok(NULL, "\n");
		break;}


	/* Verific daca linia curenta reprezinta linia "Set-cookie" pentru a extrage cookie-urile */
	if ( strstr(token, "Set-Cookie" ) ){
		
		Cookies[cookie_index] = (char*)malloc(sizeof(char)*strlen(token));
		memcpy(Cookies[cookie_index],token+12, strlen(token));
		cookie_index++;
	 	cookie_length+=strlen(token);
			
		}

	token = strtok(NULL, "\n");


     }

	
     /* Concatenam cookie-urile */
     cookie = (char*)malloc((cookie_length+7)*sizeof(char));
     sprintf(cookie, "Cookie: ");
     index = cookie+7;
     for( i=0; i<cookie_count; i++ ){
	
	char* cookie_token = strtok(Cookies[i], ";");
	if ( i<cookie_count-1){
	sprintf(index, "%s;", cookie_token );
	index+=1+strlen(cookie_token);}
	else {
	sprintf(index, "%s", cookie_token );
	index+=strlen(cookie_token);
	}
        
	
     }


     /* Eliberam memoria cookie-urilor */
     for ( i=0; i<cookie_count; i++ )
	free(Cookies[i]);
     free(Cookies);

    
    /* Parsare response */
    JSON_Value  *enunt_task4 = json_parse_string(token);
    JSON_Object *obj_task4 = json_value_get_object(enunt_task4);


    /* Extragere din campurile url, method, type - daca e cazul */
    const char* url_task4 =  json_object_get_string (obj_task4, "url");
    const char* metoda_task4 =  json_object_get_string (obj_task4, "method");

   

    /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(host_ip, portno, ip_type, socket_type, 0);

    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    if ( !strcmp(metoda_task4, "GET" )){
    	message = compute_get_request(host_ip, url_task4, NULL,cookie, auth);}

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    free(response);
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);
   
    free(cookie);
    
    /* ------------------- TASK5 ------------------- */


    /* Numara cookie-uri */
    cookie_count = 0;
    aux = response;
    while ( aux = strstr(aux, "Set-Cookie")){
	cookie_count++;
	aux++;
    }
   
    /* Aloca dinamic memorie pentru a stoca liniile ce contin cookie-uri */
    Cookies = NULL;
    if ( cookie_count ) 
	Cookies = (char**)malloc(sizeof(char*)*cookie_count);
    


    /* Parsez liniile */
    token = NULL;
    token = strtok(response, "\n");
    cookie_index = 0;
    cookie_length = 0;
    while(token ) {
	
	/* Parcurg liniile pana la prima linie goala */
	if(strlen(token)==1){
		token = strtok(NULL, "\n");
		break;}


	/* Verific daca linia curenta reprezinta linia "Set-cookie" pentru a extrage cookie-urile */
	if ( strstr(token, "Set-Cookie" ) ){
		
		Cookies[cookie_index] = (char*)malloc(sizeof(char)*strlen(token));
		memcpy(Cookies[cookie_index],token+12, strlen(token));
		cookie_index++;
	 	cookie_length+=strlen(token);
			
		}

	token = strtok(NULL, "\n");


     }

	
     /* Concatenam cookie-urile */
     cookie = (char*)malloc((cookie_length+7)*sizeof(char));
     sprintf(cookie, "Cookie: ");
     index = cookie+7;
     for( i=0; i<cookie_count; i++ ){
	
	char* cookie_token = strtok(Cookies[i], ";");
	if ( i<cookie_count-1){
	sprintf(index, "%s;", cookie_token );
	index+=1+strlen(cookie_token);}
	else {
	sprintf(index, "%s", cookie_token );
	index+=strlen(cookie_token);
	}
        
	
     }


     /* Eliberam memoria cookie-urilor */
     for ( i=0; i<cookie_count; i++ )
	free(Cookies[i]);
     free(Cookies);

    
    /* Parsare response */
    JSON_Value  *enunt_task5 = json_parse_string(token);
    JSON_Object *obj_task5 = json_value_get_object(enunt_task5);


    /* Extragere campuri */
    const char* url_task5 =  json_object_get_string (obj_task5, "url");
    const char* metoda_post5 =  json_object_get_string (obj_task5, "method");
    const char* type_task5 = json_object_get_string (obj_task5, "type");


   /* Extragere din campul data - daca e cazul */
   JSON_Object* data_task5 = json_object_get_object (obj_task5, "data");

   const char* metoda_get5 =  json_object_get_string (data_task5, "method");
   const char* adr =  json_object_get_string (data_task5, "url");

   JSON_Object* query_params5 = json_object_get_object (data_task5, "queryParams");

   /* GET params */
   
   int params_count = json_object_get_count (query_params5);
			
   char** params = (char**)malloc(sizeof(char*)*params_count);
   int params_size = 0;
   for ( i=0; i<params_count; i++ ){

	const char* param_key = json_object_get_name(query_params5, i);
	const char* param_value = json_object_get_string(query_params5, param_key);
	params[i] = (char*)malloc(sizeof(char)*(strlen(param_key)+strlen(param_value)+2));
	sprintf(params[i],"%s=%s",param_key, param_value);
   	params_size+=strlen(param_key)+strlen(param_value)+2;
				
   }

   /*Concatenare params*/
   param = (char*)malloc(sizeof(char)*(params_size+params_count-1) );
   char* p = param;
   for( i=0; i<params_count; i++ ){
	
	if(i<params_count-1){
	sprintf(p, "%s&", params[i] );
	p+=strlen(params[i])+1;}
	else{
	sprintf(p, "%s", params[i] );
	p+=strlen(params[i]);
	}
	
    }
			
   /*Free memory*/
   for ( i=0; i<params_count; i++)
		free(params[i]);
   free(params);

   
   /* Parsez adresa in ip/url */
   char* url_get5 = strstr(adr, "/");
   int len = url_get5-adr;
   char* host = (char*)malloc(sizeof(char)*len+1);
   memset(host, 0, len+1);
   memcpy(host,adr, len);
   char ip[20];
   get_ip(host,ip);

   /* Compute get message */
   /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(ip, 80, ip_type, socket_type, 0);

    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    message = compute_get_request(ip, url_get5, param ,NULL, NULL);

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    free(response);
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);
   
    
    /*GET BODY from response*/
    token = NULL;
    token = strtok(response, "\n");
    while(token ) {
	
	/* Parcurg liniile pana la prima linie goala */
	if(strlen(token)==1){
		token = strtok(NULL, "\n");
		break;}

		
	token = strtok(NULL, "\n");
     }

   
    /* Compute post message */

    /*Pas 1: Se deschide conexiunea (open_connection) */
    sockfd =  open_connection(host_ip, portno, ip_type, socket_type, 0);

    /*Pas 2: Se creaza mesajul de request (compute_get_request)*/
    message = compute_post_request(host_ip, url_task5,type_task5, token, cookie,auth);

    /*Pas 3: Se trimite la server mesajul (send_to_server)*/
    send_to_server(sockfd, message);
    free(message);

    /*Pas 4: Se primeste raspuns de la server (receive_from_server) */
    free(response);
    response = receive_from_server(sockfd);

    /*Pas 5: Se inchide conexiunea cu serverul (close_connection)*/
    close_connection(sockfd);

    printf("%s\n",response);
 
    free(param);
    free(response);
    free(cookie);
    free(host);
    json_value_free (enunt_task3);
    json_value_free (enunt_task4);
    json_value_free (enunt_task5);
    return 0;
}
