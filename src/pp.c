/* Copyright (c) 2010-2018 the corto developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <driver.tool.pp>
#include <corto.g>
#include <corto.util.argparse>

static ut_ll silent, mute, attributes, names, generators, scopes, path;
static ut_ll objects, languages, includes, imports, private_imports;
static corto_string name = NULL;
static ut_ll c4cpp = NULL;

static
int16_t cortotool_language(
    char *language)
{
    if (!generators) {
        generators = ut_ll_new();
    }

    if (!attributes) {
        attributes = ut_ll_new();
    }

    if (!strcmp(language, "c")) {
        if (!c4cpp) {
            ut_ll_append(generators, "c.project");
            ut_ll_append(generators, "c.type");
            ut_ll_append(generators, "c.interface");
            ut_ll_append(generators, "c.load");
            ut_ll_append(generators, "c.api");
            ut_ll_append(generators, "c.binding");
            ut_ll_append(generators, "c.cpp");
            ut_ll_append(attributes, "c=src");
            ut_ll_append(attributes, "h=include");
        } else {
            ut_throw("'c4cpp' can only be specified for C++ projects");
            goto error;
        }
    } else if (!strcmp(language, "cpp")) {
        if (c4cpp) {
            ut_ll_append(generators, "c.project");
            ut_ll_append(generators, "c.type");
            ut_ll_append(generators, "c.interface");
            ut_ll_append(generators, "c.load");
            ut_ll_append(generators, "c.api");
            ut_ll_append(generators, "c.binding");
            ut_ll_append(generators, "c.cpp");
            ut_ll_append(attributes, "c=src");
            ut_ll_append(attributes, "cpp=src");
            ut_ll_append(attributes, "h=include");
            ut_ll_append(attributes, "c4cpp=true");
        } else {
            ut_ll_append(generators, "c.type");
            ut_ll_append(generators, "c.load");
            ut_ll_append(generators, "c.project");
            ut_ll_append(generators, "cpp.class");
            ut_ll_append(attributes, "c=src");
            ut_ll_append(attributes, "h=include");
            ut_ll_append(attributes, "cpp=src");
            ut_ll_append(attributes, "hpp=include");
            ut_ll_append(attributes, "c4cpp=true");
            ut_ll_append(attributes, "lang=cpp");
        }
    } else {
        ut_error("unknown language '%s'", language);
        goto error;
    }

    return 0;
error:
    return -1;
}

static
int16_t cortotool_core(void) {
    ut_proc pid;
    corto_int8 ret = 0;

    pid = ut_proc_run("corto", (const char*[]){
      "corto",
      "pp",
      "--name", "corto",
      "--scope", "corto/vstore",
      "--attr", "c=src/vstore",
      "--attr", "h=include/vstore",
      "--attr", "bootstrap=true",
      "--attr", "stubs=false",
      "-g", "c.interface",
      "-g", "c.type",
      "-g", "c.binding",
      NULL
    });
    if (ut_proc_wait(pid, &ret) || ret) {
        ut_error("failed to generate code for corto/vstore (%d)", ret);
        printf("   command: corto pp --name corto --scope corto/vstore --attr c=src/core --attr h=include/core --attr bootstrap=true --attr stubs=false -g c.interface -g c.api -g c.type\n");
        goto error;
    }

    pid = ut_proc_run("corto", (const char*[]){
      "corto",
      "pp",
      "--name", "corto",
      "--scope", "corto/lang",
      "--attr", "c=src/lang",
      "--attr", "h=include/lang",
      "--attr", "bootstrap=true",
      "--attr", "stubs=false",
      "-g", "c.interface",
      "-g", "c.type",
      "-g", "c.binding",
      NULL
    });
    if (ut_proc_wait(pid, &ret) || ret) {
        ut_error("failed to generate code for corto/lang (%d)", ret);
        printf("   command: corto pp --name corto --scope corto/lang --attr c=src/lang --attr h=include/lang --attr bootstrap=true --attr stubs=false -g c.interface -g c.api -g c.type\n");
        goto error;
    }

    pid = ut_proc_run("corto", (const char*[]){
      "corto",
      "pp",
      "--name", "corto",
      "--scope", "corto/native",
      "--attr", "c=src/native",
      "--attr", "h=include/native",
      "--attr", "bootstrap=true",
      "--attr", "stubs=false",
      "-g", "c.interface",
      "-g", "c.type",
      "-g", "c.binding",
      NULL
    });
    if (ut_proc_wait(pid, &ret) || ret) {
        ut_error("failed to generate code for corto/native (%d)", ret);
        printf("   command: corto pp --name corto --scope corto/native --attr c=src/native --attr h=include/native --attr bootstrap=true --attr stubs=false -g c.interface -g c.api -g c.type\n");
        goto error;
    }

    pid = ut_proc_run("corto", (const char*[]){
      "corto",
      "pp",
      "--name", "corto",
      "--scope", "corto/secure",
      "--attr", "c=src/secure",
      "--attr", "h=include/secure",
      "--attr", "bootstrap=true",
      "--attr", "stubs=false",
      "-g", "c.interface",
      "-g", "c.type",
      "-g", "c.binding",
      NULL
    });
    if (ut_proc_wait(pid, &ret) || ret) {
        ut_error("failed to generate code for corto/native (%d)", ret);
        printf("   command: corto pp --name corto --scope corto/secure --attr c=src/secure --attr h=include/secure --attr bootstrap=true --attr stubs=false -g c.interface -g c.api -g c.type\n");
        goto error;
    }

    /* Generate C API */
    pid = ut_proc_run("corto", (const char*[]){
      "corto",
      "pp",
      "--name", "corto",
      "--scope", "corto/lang,corto/vstore,corto/secure,corto/native",
      "--attr", "c=src",
      "--attr", "h=include",
      "--attr", "bootstrap=true",
      "--attr", "stubs=false",
      "-g", "c.api",
      NULL
    });
    if (ut_proc_wait(pid, &ret) || ret) {
        ut_error("failed to generate code for corto/c (%d)", ret);
        printf("   command: corto pp --name corto --scope corto/vstore --attr c=src/core --attr h=include/core --attr bootstrap=true --attr stubs=false -g c.interface -g c.api -g c.type\n");
        goto error;
    }

    return 0;
error:
    return -1;
}

static
int16_t cortotool_ppParse(
    g_generator g,
    ut_ll list,
    const char *projectName,
    corto_bool parseSelf,
    corto_bool parseScope)
{
    ut_iter it = ut_ll_iter(list);
    while (ut_iter_hasNext(&it)) {
        corto_id id;
        char *objId = ut_iter_next(&it);

        /* Ensure the scope is fully qualified */
        if ((objId[0] != '/') && (objId[0] != ':')) {
            sprintf(id, "/%s", objId);
        } else {
            strcpy(id, objId);
        }

        /* Resolve object */
        corto_object o = corto_resolve(NULL, id);
        if (!o) {
            if (includes && ut_ll_count(includes)) {
                if (ut_ll_count(includes) == 1) {
                    ut_error("'%s' does not define object '%s'",
                      ut_ll_get(includes, 0),
                      id);
                } else {
                    ut_error("object '%s' is not in loaded definitions",
                      id);
                }
            } else {
                ut_error("unresolved object '%s' (did you forget to load the definitions?)", id);
            }
            goto error;
        }

        g_parse(g, o, parseSelf, parseScope);

        corto_release(o);
    }

    return 0;
error:
    return -1;
}

static
int16_t cortotool_ppLoadImports(
    g_generator g,
    ut_ll imports,
    bool private)
{
    ut_iter it = ut_ll_iter(imports);
    while (ut_iter_hasNext(&it)) {
        corto_string import = ut_iter_next(&it);

        /* Don't load corto */
        if (!strcmp(import, "corto") ||
            !strcmp(import, "/corto"))
        {
            continue;
        }

        /* Check if package exists */
        const char *libpath = ut_locate(
            import, NULL, UT_LOCATE_PROJECT);
        if (!libpath) {
            ut_throw("package '%s' not found", import);
            goto error;
        }

        /* Check if package has a loadable library */
        const char *lib = ut_locate(import, NULL, UT_LOCATE_LIB);
        const char *include = ut_locate(import, NULL, UT_LOCATE_INCLUDE);
        corto_package package = NULL;

        if (lib) {
            /* Fetch library */
            ut_dl dl = NULL;
            void *proc = ut_load_sym(import, &dl, "cortomain");
            if (!dl) {
                ut_throw("loading package '%s' failed", import);
                goto error;
            } else if (!proc) {
                /* Catch error thrown by ut_load_sym */
                ut_catch();
            }

            /* If lib has cortomain, check if a package object was created */
            if (proc) {
                /* Import package */
                if (ut_use(import, 0, NULL)) {
                    ut_throw("loading model for package '%s' failed ", import);
                    goto error;
                }

                package = corto_lookup(NULL, import);
            }
        }

        if (lib || include) {
            if (!package) {
                /* Package did not create an object for itself. Create dummy
                 * package object to pass to generator */
                package = corto_create(root_o, import, corto_package_o);
            }

            if (private) {
                if (g_import_private(g, package)) {
                    goto error;
                }
            } else {
                if (g_import(g, package)) {
                    goto error;
                }
            }
        }
    }

    return 0;
error:
    return -1;
}

/* Enter package initialization code here */
int cortomain(int argc, char *argv[]) {
    g_generator g;
    corto_string lib, include;
    ut_iter it;
    corto_string attr;
    ut_ll core;
    char *language = NULL;

    CORTO_UNUSED(argc);

    /* If a model contains a package that is not in the imports
     * specified on the commandline, don't automatically load it, but throw an
     * error. This guarantees that the model cannot use packages that
     * are not part of the project dependencies. */
    corto_enable_load(false);
    corto_autoload(false);

    ut_ok("corto preprocessor v1.0");

    corto_argdata *data = corto_argparse(
      argv,
      (corto_argdata[]){
        {"$0", NULL, NULL}, /* Ignore 'pp' */
        {"--path", NULL, &path},
        {"--silent", &silent, NULL},
        {"--mute", &mute, NULL},
        {"--core", &core, NULL},
        {"--attr", NULL, &attributes},
        {"--name", NULL, &names},
        {"--scope", NULL, &scopes},
        {"--object", NULL, &objects},
        {"--import", NULL, &imports},
        {"--use", NULL, &imports},
        {"--use-private", NULL, &private_imports},
        {"--c4cpp", &c4cpp, NULL},
        {"-s", NULL, &scopes},
        {"$+--generator", NULL, &generators},
        {"$|-g", NULL, &generators},
        {"$|--lang", NULL, &languages},
        {"$|-l", NULL, &languages},
        {"*", &includes, NULL},
        {NULL}
      }
    );

    if (path) {
        const char *cur_path = ut_ll_get(path, 0);
        ut_chdir(cur_path);
    }

    if (!data) {
        ut_throw(NULL);
        goto error;
    }

    if (names) {
        name = ut_ll_get(names, 0);
    }

    if (core) {
        ut_trace("regenerate core");
        return cortotool_core();
    }

    if (languages) {
        language = ut_ll_get(languages, 0);
        cortotool_language(language);
    }

    ut_trace("start generator from '%s'", ut_cwd());

    g = g_new(name, NULL);

    /* Load imports */
    ut_log_push("import");
    if (imports) {
        if (cortotool_ppLoadImports(g, imports, false)) {
            goto error;
        }
    }
    if (private_imports) {
        if (cortotool_ppLoadImports(g, private_imports, true)) {
            goto error;
        }
    }
    ut_log_pop();

    /* Create marker object that will tell generation framework which objects
     * were created by code generator  */
    corto_object marker = corto_create(root_o, "pp_marker", corto_void_o);

    /* Set marker object as source */
    corto_object prev = corto_set_source(marker);

    /* Load includes */
    if (includes) {
        ut_log_push("model");
        it = ut_ll_iter(includes);
        while (ut_iter_hasNext(&it)) {
            include = ut_iter_next(&it);

            ut_trace("loading '%s'", include);
            if (ut_use(include, 0, NULL)) {
                ut_throw("failed to load '%s'", include);
                goto error;
            } else {
                /* Add name to scope list if none provided */
                if (!scopes && (ut_ll_count(includes) == 1) && !strchr(include, '.')) {
                    scopes = ut_ll_new();
                    ut_ll_insert(scopes, include);
                }
            }
        }

        /* If there's a single include file, set an attribute to pass the name
         * of the file to a generator */
        if (ut_ll_count(includes) == 1) {
            corto_string str = ut_asprintf("include=%s", ut_ll_get(includes, 0));
            if (!attributes) {
                attributes = ut_ll_new();
            }
            ut_ll_append(attributes, str);
        }
        ut_log_pop();
    }

    corto_set_source(prev);

    /* Load library */
    if (generators) {
        ut_log_push("gen");

        /* Generate for all scopes */
        if (scopes) {
            if (cortotool_ppParse(g, scopes, name, TRUE, TRUE)) {
                ut_log_pop();
                goto error;
            }
        }
        if (objects) {
            if (cortotool_ppParse(g, objects, name, TRUE, FALSE)) {
                ut_log_pop();
                goto error;
            }
        }

        /* Set attributes */
        if (attributes) {
            it = ut_ll_iter(attributes);
            while (ut_iter_hasNext(&it)) {
                corto_string ptr;

                attr = ut_strdup(ut_iter_next(&it));

                ptr = strchr(attr, '=');
                if (ptr) {
                    *ptr = '\0';
                    g_setAttribute(g, attr, ptr+1);
                }
                *ptr = '=';
                corto_dealloc(attr);
            }
        }

        /* Parse for every specified generator */
        while ((lib = ut_ll_takeFirst(generators))) {
            ut_log_push(lib);

            /* Load generator package */
            if (g_load(g, lib)) {
                ut_log_pop();
                ut_throw("generator '%s' failed to load", lib);
                ut_log_pop();
                goto error;
            }

            /* Start generator */
            ut_trace("run generator '%s'", lib);
            if (g_start(g)) {
                ut_log_pop();
                ut_throw("generator '%s' failed to run", lib);
                g_free(g);
                ut_log_pop();
                goto error;
            }

            ut_log_pop();
        }

        ut_log_pop();
    }

    g_free(g);

    /* Cleanup application resources */
    corto_argclean(data);

    ut_ok("done");

    return 0;
error:
    corto_set_source(NULL);
    return -1;
}
