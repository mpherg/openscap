

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oval_probe.h>
#include <oval_agent_api.h>

int _test_error_handler(struct oval_xml_error *error, void *null)
{
        //ERROR HANDLING IS TODO
        return 1;
}


int main(int argc, char **argv)
{
	/* definition model populate */
        struct oval_import_source *def_in = oval_import_source_new_file(argv[1]);
        struct oval_definition_model *def_model = oval_definition_model_new();
        oval_definition_model_import(def_model, def_in, &_test_error_handler, NULL);
        oval_import_source_free(def_in);

	/* create syschar model */
	struct oval_syschar_model *sys_model = oval_syschar_model_new(def_model);

	/* probe objects in tests */
	oval_syschar_model_get_sysinfo(sys_model);
	struct oval_definition_model *definition_model = oval_syschar_model_get_definition_model(sys_model); /* needed? */
	struct oval_test * test;
	struct oval_object *object;
	char *objid;
	struct oval_syschar *syschar;
	oval_pctx_t *pctx = oval_pctx_new (sys_model);
	struct oval_test_iterator * test_it = oval_definition_model_get_tests(definition_model);
	while( oval_test_iterator_has_more(test_it) ) {
		test = oval_test_iterator_next(test_it);
		object = oval_test_get_object(test);
		objid = oval_object_get_id(object);
                syschar = oval_syschar_model_get_syschar(sys_model, objid);
                if(syschar==NULL) {
	                syschar = oval_probe_object_eval (pctx, object);
	                if(syschar == NULL) {
        	                syschar = oval_syschar_new(sys_model, object);
 	                        oval_syschar_set_flag(syschar,SYSCHAR_FLAG_NOT_COLLECTED);
                        }
                        oval_syschar_model_add_syschar(sys_model, syschar);
                }
	}
        oval_pctx_free (pctx);
        oval_test_iterator_free(test_it);


	/* print # syschars */
        int count = 0;
        struct oval_syschar_iterator *syschars = oval_syschar_model_get_syschars(sys_model);
        for (count = 0; oval_syschar_iterator_has_more(syschars); count++) {
        	oval_syschar_iterator_next(syschars);
        }
        oval_syschar_iterator_free(syschars);
	printf("THERE ARE %d SYSCHARS\n", count);

	/* report syschar into xml file */
	char * syschar_fname = malloc( sizeof(char) * strlen(argv[1]) + 15 );
	sprintf(syschar_fname, "%s.syschar.xml",argv[1]);
	struct oval_export_target *syschar_out  = oval_export_target_new_file(syschar_fname, "UTF-8");
	oval_syschar_model_export(sys_model, syschar_out);
	free(syschar_fname);
	oval_export_target_free(syschar_out);

	/* create result model */
	struct oval_syschar_model *sys_models[] = {sys_model, NULL};
        struct oval_results_model* res_model = oval_results_model_new( def_model, sys_models );

	/* set up directives */
	struct oval_result_directives * res_direct = oval_result_directives_new(res_model);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_INVALID, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_TRUE, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_FALSE, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_UNKNOWN, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_ERROR, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_NOT_EVALUATED, true);
	oval_result_directives_set_reported(res_direct, OVAL_RESULT_NOT_APPLICABLE , true);
	oval_result_directives_set_content(res_direct,OVAL_RESULT_FALSE, OVAL_DIRECTIVE_CONTENT_FULL);
	oval_result_directives_set_content(res_direct,OVAL_RESULT_TRUE, OVAL_DIRECTIVE_CONTENT_FULL);

	/* report results */
	char * results_fname = malloc( sizeof(char) * strlen(argv[1]) + 15 );
	sprintf(results_fname, "%s.results.xml",argv[1]);
	struct oval_export_target *result_out  = oval_export_target_new_file(results_fname, "UTF-8");
	oval_results_model_export(res_model, res_direct, result_out);
	free(results_fname);
	oval_export_target_free(result_out);

	oval_definition_model_free(def_model);
	oval_syschar_model_free(sys_model);
	oval_results_model_free(res_model);
	
        return 0;
}

