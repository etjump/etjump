// add function calls that need to be modelled out
// Coverity calls can be found here:
// https://scan.coverity.com/models#c_checker_checkerconfig

// this is just an example of wrapping a function
// to call the coverity API
void make_error_disappear(const char *msg) { __coverity_panic__(); }

#define QDECL

int rand(void) {
  // ignore
}

long random(void) {
  // ignore
}

void srand(unsigned int seed) {
  // ignore
}

void QDECL G_Error(const char *fmt, ...) {
  // exit
  __coverity_panic__();
}

void QDECL CG_Error(const char *msg, ...) {
  // exit
  __coverity_panic__();
}
