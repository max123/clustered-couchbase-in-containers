#include <libcouchbase/couchbase.h>
#include <libcouchbase/n1ql.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * to run, ./hellocb couchbase://192.168.129.99/benchmark
 * or ./hellocb couchbase://localhost/benchmark
 * or use the ip and bucket of your choice:
 * ./hellocb couchbase://some_ip_or_localhost/bucket
 */

static void 
rowCallback(lcb_t instance, int cbtype, const lcb_RESPN1QL *resp)
{
	if (! (resp->rflags & LCB_RESP_F_FINAL)) {
		printf("Row: %.*s\n", (int)resp->nrow, resp->row);
	} else {
		printf("Got metadata: %.*s\n", (int)resp->nrow, resp->row);
	}
}

int
main(int argc, char **argv)
{
	char *url;
	// initializing
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s url", argv[0]);
		exit(1);
	}

	url = argv[1];
	struct lcb_create_st cropts = { 0 };
	cropts.version = 3;
/*	cropts.v.v3.connstr = "couchbase://localhost/benchmark"; */
	cropts.v.v3.connstr = url;
	lcb_error_t err;
	lcb_t instance;
	err = lcb_create(&instance, &cropts);
	if (err != LCB_SUCCESS) {
		printf("Couldn't create instance!\n");
		printf("lcb_create: %s\n", lcb_strerror(NULL, err));
		exit(1);
	}
  
	// connecting
  
	lcb_connect(instance);
	lcb_wait(instance);
	if ( (err = lcb_get_bootstrap_status(instance)) != LCB_SUCCESS ) {
		printf("Couldn't bootstrap!\n");
		exit(1);
	}

	printf("bootstrapped...\n"); 
	// installing callbacks
  
	
	const char *querystr =
	  "{"
		  "\"statement\":\"SELECT id, name FROM benchmark WHERE email = '00000000000000000004@joyent.com'\""
		  "}";

	printf("querystr = %s\n", querystr);
	lcb_CMDN1QL cmd = { 0 };
	cmd.query = querystr;
	cmd.nquery = strlen(querystr);
	cmd.host = "192.168.129.99:8093";
	cmd.callback = rowCallback;
	lcb_error_t rc = lcb_n1ql_query(instance, NULL, &cmd);
	fprintf(stderr, "lcb_n1ql_query: %s\n", lcb_strerror(NULL, rc));
	lcb_wait(instance);
	lcb_destroy(instance);
	return 0;
}
