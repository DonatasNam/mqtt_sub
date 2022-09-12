#include "email.h"

char *ops_to_string(int operator){

	switch (operator)
	{
	case 10:
		return ">";
		break;
	case 11:
		return "<";
		break;
	case 12:
		return "=";
		break;
	case 13:
		return "!=";
		break;
	case 14:
		return ">=";
		break;
	case 115:
		return "<=";
		break;	
	default:
		return NULL;
		syslog(LOG_ERR,"invalid operator, err: %d", INVALID_OP);
		break;
	}
}

static char *mail_message(char *recipient, char *email, char *subject, char *msg){

	char *message = (char *) malloc(sizeof(char) * (strlen(msg) + 
    strlen(recipient) + strlen(email) + strlen(subject) + 40));

    if ( message == NULL ){
		syslog(LOG_ERR,"Failed to form an email message");
        return NULL;
    }

    sprintf(message,
    "To: <%s> \r\n"
    "From: <%s> \r\n"
    "Subject: %s\r\n"
    "\r\n" 
    "%s\r\n"
    "\r\n"
    , recipient,email, subject, msg);

	return message;
}


 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp){
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    size_t room = size * nmemb;
  
    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
      return 0;
    }
  
    //data = &payload_text[upload_ctx->bytes_read];
    data = &upload_ctx->email_msg[upload_ctx->bytes_read];
  
    if(data) {
      size_t len = strlen(data);
      if(room < len){
        len = room;
    }
      memcpy(ptr, data, len);
      upload_ctx->bytes_read += len;

      return len;
    }
}

int send_mail(event *ev_head,char *payload){
	
	int rc =0;
  	CURL *curl;
  	CURLcode res = CURLE_OK;
  	struct curl_slist *recipients = NULL;
 	struct upload_status upload_ctx;
	char *subject ="Messagge from mqtt subscriber";
	char *msg =(char *)malloc(strlen(payload)+strlen(ev_head->value)+strlen(ev_head->key)+40);
	sprintf(msg,"Event triggered %s %s %s\n payload: %s",ev_head->key,ops_to_string(ev_head->operator),ev_head->value,payload);
  	upload_ctx.bytes_read = 0;
	upload_ctx.email_msg =mail_message(ev_head->recipient,ev_head->email,subject,msg);

  	curl = curl_easy_init();
  	if(!curl) {
		rc =CURLE_FAILED_INIT;
		syslog(LOG_ERR, "curl_easy_perform() failed: %s, err: %d\n", curl_easy_strerror(rc), rc);
		return rc;
  	}
    curl_easy_setopt(curl, CURLOPT_USE_SSL, 3L);
    curl_easy_setopt(curl, CURLOPT_CAINFO,SSL_CERT_PATH);
    curl_easy_setopt(curl, CURLOPT_URL, ev_head->smtp_server);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ev_head->email);
    recipients = curl_slist_append(recipients, ev_head->recipient);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_USERNAME, ev_head->email);
	curl_easy_setopt(curl,CURLOPT_PASSWORD, ev_head->password);

    rc = curl_easy_perform(curl);
 
    /* Check for errors */ 
    if(rc != CURLE_OK){
    	syslog(LOG_ERR, "curl_easy_perform() failed: %s, err: %d\n", curl_easy_strerror(rc), rc);
	}
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
	free(msg);
	free(upload_ctx.email_msg);
  	return (int)rc;
}