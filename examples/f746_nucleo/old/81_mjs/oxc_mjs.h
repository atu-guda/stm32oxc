// based on https://github.com/cesanta/mjs
// mjs.h mod by atu:
#pragma once

#define _GNU_SOURCE 1

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <vector>

namespace OXC_MJS // for future
{
};



#if !defined(PRINTF_LIKE)
#if defined(__GNUC__) || defined(__clang__) || defined(__TI_COMPILER_VERSION__)
#define PRINTF_LIKE(f, a) __attribute__((format(printf, f, a)))
#else
#define PRINTF_LIKE(f, a)
#endif
#endif

#define WEAK __attribute__((weak))

#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#define NOINLINE __attribute__((noinline))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define NOINSTR __attribute__((no_instrument_function))
#define DO_NOT_WARN_UNUSED __attribute__((unused))
#else
#define NORETURN
#define NOINLINE
#define WARN_UNUSED_RESULT
#define NOINSTR
#define DO_NOT_WARN_UNUSED
#endif /* __GNUC__ */


//namespace OXC_MJS
//{


/*
 *  Double-precision floating-point number, IEEE 754
 *
 *  64 bit (8 bytes) in total
 *  1  bit sign
 *  11 bits exponent
 *  52 bits mantissa
 *      7         6        5        4        3        2        1        0
 *  seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
 *
 * If an exponent is all-1 and mantissa is all-0, then it is an INFINITY:
 *  11111111|11110000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 * If an exponent is all-1 and mantissa's MSB is 1, it is a quiet NaN:
 *  11111111|11111000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 *  MJS NaN-packing:
 *    sign and exponent is 0xfff
 *    4 bits specify type (tag), must be non-zero
 *    48 bits specify value
 *
 *  11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 *   NaN marker |type|  48-bit placeholder for values: pointers, strings
 *
 * On 64-bit platforms, pointers are really 48 bit only, so they can fit,
 * provided they are sign extended
 */

typedef uint64_t mjs_val_t;

class Mjs;

/*
 * Log level; `LL_INFO` is the default. Use `cs_log_set_level()` to change it.
 */
enum Cs_log_level {
  LL_NONE = -1,
  LL_ERROR = 0,
  LL_WARN = 1,
  LL_INFO = 2,
  LL_DEBUG = 3,
  LL_VERBOSE_DEBUG = 4,

  _LL_MIN = -2,
  _LL_MAX = 5,
};
/*
 * Set max log level to print; messages with the level above the given one will
 * not be printed.
 */
void cs_log_set_level( Cs_log_level level );
/*
 * A comma-separated set of prefix=level.
 * prefix is matched against the log prefix exactly as printed, including line
 * number, but partial match is ok. Check stops on first matching entry.
 * If nothing matches, default level is used.
 *
 * Examples:
 *   main.c:=4 - everything from main C at verbose debug level.
 *   mongoose.c=1,mjs.c=1,=4 - everything at verbose debug except mg_* and mjs_*
 *
 */
void cs_log_set_file_level( const char *file_level );
/*
 * Helper function which prints message prefix with the given `level`.
 * If message should be printed (according to the current log level
 * and filter), prints the prefix and returns 1, otherwise returns 0.
 *
 * Clients should typically just use `LOG()` macro.
 */
int cs_log_print_prefix( Cs_log_level level, const char *fname, int line );
/*
 * Prints log to the current log file, appends "\n" in the end and flushes the
 * stream.
 */
void cs_log_printf(const char *fmt, ...) PRINTF_LIKE(1, 2);

enum mjs_err_t {
  MJS_OK,
  MJS_SYNTAX_ERROR,
  MJS_REFERENCE_ERROR,
  MJS_TYPE_ERROR,
  MJS_OUT_OF_MEMORY,
  MJS_INTERNAL_ERROR,
  MJS_NOT_IMPLEMENTED_ERROR,
  MJS_FILE_READ_ERROR,
  MJS_BAD_ARGS_ERROR,

  MJS_ERRS_CNT
};

/* Describes chunk of memory */
struct Mg_str {
  Mg_str() : p( nullptr ), len( 0 ) {};
  explicit Mg_str( const char *str );
  Mg_str( const char *s, size_t l ) : p( s ), len( l ) {};
  void clear() { p = nullptr; len = 0; };
  const char* strchr( int c ) const;
  void strfree(); // may be unused
  const char* strstr( const Mg_str &needle ) const;
  void strstrip();
  bool starts_with( const Mg_str &prefix ) const;
  friend int mg_strcmp( const Mg_str &str1, const Mg_str &str2 );
  friend int mg_vcmp( const Mg_str &str1, const char *str2 );
  friend int mg_vcasecmp( const Mg_str &str1, const char *str2 );
  friend Mg_str mg_strdup_common( const Mg_str &s, int nul_terminate );
  friend int mg_strncmp( const Mg_str &str1, const Mg_str &str2, size_t n );
  friend int mg_strcasecmp( const Mg_str &str1, const Mg_str &str2 );

  const char *p; /* Memory chunk pointer */
  size_t len;    /* Memory chunk length */
};

// ------------------------ Mbuf  -> Xbuf ------------------------------------

using Xbuf = std::vector<char>;

/* Memory buffer descriptor */
struct Mbuf {
  char *buf;   /* Buffer pointer */
  size_t len;  /* Data length. Data is located between offset 0 and len. */
  size_t size; /* Buffer size allocated by realloc(1). Must be >= len */
};

// ------------------------ Mjs  ------------------------------------

struct mjs_vals {
  /* Current `this` value  */
  mjs_val_t this_obj;
  mjs_val_t dataview_proto;

  /*
   * The object against which the last `OP_GET` was invoked. Needed for
   * "method invocation pattern".
   */
  mjs_val_t last_getprop_obj;
};

enum mjs_ffi_ctype {
  MJS_FFI_CTYPE_NONE,
  MJS_FFI_CTYPE_USERDATA,
  MJS_FFI_CTYPE_CALLBACK,
  MJS_FFI_CTYPE_INT,
  MJS_FFI_CTYPE_BOOL,
  MJS_FFI_CTYPE_DOUBLE,
  MJS_FFI_CTYPE_FLOAT,
  MJS_FFI_CTYPE_CHAR_PTR,
  MJS_FFI_CTYPE_VOID_PTR,
  MJS_FFI_CTYPE_STRUCT_MG_STR_PTR,
  MJS_FFI_CTYPE_STRUCT_MG_STR,
  MJS_FFI_CTYPE_INVALID,
};

typedef void *(mjs_ffi_resolver_t)(void *handle, const char *symbol);

void mjs_set_ffi_resolver(Mjs *mjs, mjs_ffi_resolver_t *dlsym);

typedef uint8_t mjs_ffi_ctype_t;

enum ffi_sig_type {
  FFI_SIG_FUNC,
  FFI_SIG_CALLBACK,
};

#define MJS_CB_ARGS_MAX_CNT 6
#define MJS_CB_SIGNATURE_MAX_SIZE (MJS_CB_ARGS_MAX_CNT + 1 /* return type */)

/*
 * Maximum number of word-sized args to ffi-ed function. If at least one
 * of the args is double, only 2 args are allowed.
 */
#define FFI_MAX_ARGS_CNT 6

typedef void(ffi_fn_t)(void);

typedef intptr_t ffi_word_t;

enum ffi_ctype {
  FFI_CTYPE_WORD,
  FFI_CTYPE_BOOL,
  FFI_CTYPE_FLOAT,
  FFI_CTYPE_DOUBLE,
};

struct ffi_arg {
  enum ffi_ctype ctype;
  union {
    uint64_t i;
    double d;
    float f;
  } v;
};

//* Parsed FFI signature
struct mjs_ffi_sig {
  /*
   * Callback signature, corresponds to the arg of type MJS_FFI_CTYPE_CALLBACK
   * TODO(dfrank): probably we'll need to support multiple callback/userdata
   * pairs
   *
   * NOTE(dfrank): instances of this structure are grouped into GC arenas and
   * managed by GC, and for the GC mark to work, the first element should be
   * a pointer (so that the two LSBs are not used).
   */
  mjs_ffi_sig *cb_sig;

  /*
   * The first item is the return value type (for `void`, `MJS_FFI_CTYPE_NONE`
   * is used); the rest are arguments. If some argument is
   * `MJS_FFI_CTYPE_NONE`, it means that there are no more arguments.
   */
  mjs_ffi_ctype_t val_types[MJS_CB_SIGNATURE_MAX_SIZE];

  /*
   * Function to call. If `is_callback` is not set, then it's the function
   * obtained by dlsym; otherwise it's a pointer to the appropriate callback
   * implementation.
   */
  ffi_fn_t *fn;

  /* Number of arguments in the signature */
  int8_t args_cnt;

  /*
   * If set, then the signature represents the callback (as opposed to a normal
   * function), and `fn` points to the suitable callback implementation.
   */
  unsigned is_callback : 1;
  unsigned is_valid : 1;
};
typedef struct mjs_ffi_sig mjs_ffi_sig_t;

/* Initialize new FFI signature */
void mjs_ffi_sig_init(mjs_ffi_sig_t *sig);
/* Copy existing FFI signature */
void mjs_ffi_sig_copy(mjs_ffi_sig_t *to, const mjs_ffi_sig_t *from);
/* Free FFI signature. NOTE: the pointer `sig` itself is not freed */
void mjs_ffi_sig_free(mjs_ffi_sig_t *sig);

struct mjs_ffi_cb_args {
  struct mjs_ffi_cb_args *next;
  Mjs *mjs;
  mjs_ffi_sig_t sig;
  mjs_val_t func;
  mjs_val_t userdata;
};
typedef struct mjs_ffi_cb_args ffi_cb_args_t;

typedef void (*gc_cell_destructor_t)(Mjs *mjs, void *);

struct gc_block {
  struct gc_block *next;
  struct gc_cell *base;
  size_t size;
};

struct gc_arena {
  struct gc_block *blocks;
  size_t size_increment;
  struct gc_cell *free; /* head of free list */
  size_t cell_size;

  gc_cell_destructor_t destructor;
};

#define JUMP_INSTRUCTION_SIZE 2

enum mjs_type {
  /* Primitive types */
  MJS_TYPE_UNDEFINED,
  MJS_TYPE_NULL,
  MJS_TYPE_BOOLEAN,
  MJS_TYPE_NUMBER,
  MJS_TYPE_STRING,
  MJS_TYPE_FOREIGN,

  /* Different classes of Object type */
  MJS_TYPE_OBJECT_GENERIC,
  MJS_TYPE_OBJECT_ARRAY,
  MJS_TYPE_OBJECT_FUNCTION,
  /*
   * TODO(dfrank): if we support prototypes, need to add items for them here
   */

  MJS_TYPES_CNT
};

struct mjs_property {
  mjs_property *next; /* Linkage in struct mjs_object::properties */
  mjs_val_t name;            /* Property name (a string) */
  mjs_val_t value;           /* Property value */
};

struct mjs_object {
  mjs_property *properties;
};

enum mjs_call_stack_frame_item {
  CALL_STACK_FRAME_ITEM_RETVAL_STACK_IDX, /* TOS */
  CALL_STACK_FRAME_ITEM_LOOP_ADDR_IDX,
  CALL_STACK_FRAME_ITEM_SCOPE_IDX,
  CALL_STACK_FRAME_ITEM_RETURN_ADDR,
  CALL_STACK_FRAME_ITEM_THIS,

  CALL_STACK_FRAME_ITEMS_CNT
};


struct mjs_bcode_part {
  /* Global index of the bcode part */
  size_t start_idx;

  /* Actual bcode data */
  struct {
    const char *p; /* Memory chunk pointer */
    size_t len;    /* Memory chunk length */
  } data;

  /*
   * Result of evaluation (not parsing: if there is an error during parsing,
   * the bcode is not even committed). It is used to determine whether we
   * need to evaluate the file: if file was already evaluated, and the result
   * was MJS_OK, then we won't evaluate it again. Otherwise, we will.
   */
  mjs_err_t exec_res : 4;

  /* If set, bcode data does not need to be freed */
  unsigned in_rom : 1;
};


/*
 * Bcode header: type of the items, and item numbers.
 */
typedef uint32_t mjs_header_item_t;
enum mjs_header_items {
  MJS_HDR_ITEM_TOTAL_SIZE,   /* Total size of the bcode (not counting the
                                OP_BCODE_HEADER byte) */
  MJS_HDR_ITEM_BCODE_OFFSET, /* Offset to the start of the actual bcode (not
                                counting the OP_BCODE_HEADER byte) */
  MJS_HDR_ITEM_MAP_OFFSET,   /* Offset to the start of offset-to-line_no mapping
                                k*/

  MJS_HDR_ITEMS_CNT
};

size_t mjs_get_func_addr(mjs_val_t v);

int mjs_getretvalpos(Mjs *mjs);

enum mjs_type mjs_get_type(mjs_val_t v);

/*
 * Prints stack trace starting from the given bcode offset; other offsets
 * (if any) will be fetched from the call_stack.
 */
void mjs_gen_stack_trace(Mjs *mjs, size_t offset);

mjs_val_t vtop(Mbuf *m);
size_t mjs_stack_size(const Mbuf *m);
mjs_val_t *vptr(Mbuf *m, int idx);
void push_mjs_val(Mbuf *m, mjs_val_t v);
mjs_val_t mjs_pop_val(Mbuf *m);
mjs_val_t mjs_pop(Mjs *mjs);
void mjs_push(Mjs *mjs, mjs_val_t v);
void mjs_die(Mjs *mjs);

/*
 * A tag is made of the sign bit and the 4 lower order bits of byte 6.
 * So in total we have 32 possible tags.
 *
 * Tag (1,0) however cannot hold a zero payload otherwise it's interpreted as an
 * INFINITY; for simplicity we're just not going to use that combination.
 */
#define MAKE_TAG(s, t) \
  ((uint64_t)(s) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(t) << 48)

#define MJS_TAG_OBJECT MAKE_TAG(1, 1)
#define MJS_TAG_FOREIGN MAKE_TAG(1, 2)
#define MJS_TAG_UNDEFINED MAKE_TAG(1, 3)
#define MJS_TAG_BOOLEAN MAKE_TAG(1, 4)
#define MJS_TAG_NAN MAKE_TAG(1, 5)
#define MJS_TAG_STRING_I MAKE_TAG(1, 6)  /* Inlined string len < 5 */
#define MJS_TAG_STRING_5 MAKE_TAG(1, 7)  /* Inlined string len 5 */
#define MJS_TAG_STRING_O MAKE_TAG(1, 8)  /* Owned string */
#define MJS_TAG_STRING_F MAKE_TAG(1, 9)  /* Foreign string */
#define MJS_TAG_STRING_C MAKE_TAG(1, 10) /* String chunk */
#define MJS_TAG_STRING_D MAKE_TAG(1, 11) /* Dictionary string  */
#define MJS_TAG_ARRAY MAKE_TAG(1, 12)
#define MJS_TAG_FUNCTION MAKE_TAG(1, 13)
#define MJS_TAG_FUNCTION_FFI MAKE_TAG(1, 14)
#define MJS_TAG_NULL MAKE_TAG(1, 15)

#define MJS_TAG_MASK MAKE_TAG(1, 15)


/* JavaScript `null` value */
#define MJS_NULL MJS_TAG_NULL

/* JavaScript `undefined` value */
#define MJS_UNDEFINED MJS_TAG_UNDEFINED

constexpr uint64_t make_tag( uint64_t s, uint64_t t ) // TODO: add Msj ???
     { return s << 63 | ( (uint64_t) 0x7ff0 << 48 ) | t << 48 ; }


class Mjs {
  public:
   Mjs();
   ~Mjs();
   mjs_val_t get_global() { return *vptr( &scopes, 0 ); } // ?

   enum MsjTag {
     Mjs_TAG_OBJECT       = make_tag( 1,  1 ),
     Mjs_TAG_FOREIGN      = make_tag( 1,  2 ),
     Mjs_TAG_UNDEFINED    = make_tag( 1,  3 ),
     Mjs_TAG_BOOLEAN      = make_tag( 1,  4 ),
     Mjs_TAG_NAN          = make_tag( 1,  5 ),
     Mjs_TAG_STRING_I     = make_tag( 1,  6 ), /* Inlined string len < 5 */
     Mjs_TAG_STRING_5     = make_tag( 1,  7 ), /* Inlined string len 5 */
     Mjs_TAG_STRING_O     = make_tag( 1,  8 ), /* Owned string */
     Mjs_TAG_STRING_F     = make_tag( 1,  9 ), /* Foreign string */
     Mjs_TAG_STRING_C     = make_tag( 1, 10 ), /* String chunk */
     Mjs_TAG_STRING_D     = make_tag( 1, 11 ), /* Dictionary string  */
     Mjs_TAG_ARRAY        = make_tag( 1, 12 ),
     Mjs_TAG_FUNCTION     = make_tag( 1, 13 ),
     Mjs_TAG_FUNCTION_FFI = make_tag( 1, 14 ),
     Mjs_TAG_NULL         = make_tag( 1, 15 ),
     Mjs_TAG_MASK         = make_tag( 1, 15 ),
     Mjs_NULL             = Mjs_TAG_NULL,
     Mjs_UNDEFINED        = Mjs_TAG_UNDEFINED //* JavaScript `undefined` value
   };
   static inline bool is_array( mjs_val_t v )  { return ( v & Mjs_TAG_MASK ) == Mjs_TAG_ARRAY; }
   static inline bool is_object( mjs_val_t v )
     { return ( v & Mjs_TAG_MASK ) == Mjs_TAG_OBJECT || ( v & Mjs_TAG_MASK) == Mjs_TAG_ARRAY; }

   mjs_val_t mk_array();
   mjs_val_t mk_object();  //* Make an empty object
   mjs_err_t set( mjs_val_t obj, const char *name, size_t name_len, mjs_val_t val );
   mjs_err_t set_v( mjs_val_t obj, mjs_val_t name, mjs_val_t val );

   int get_offset_by_call_frame_num( int cf_num ); // const?

  private:
   mjs_err_t set_internal( mjs_val_t obj, mjs_val_t name_v, char *name, size_t name_len, mjs_val_t val );

  public:
   // TODO: make most of them private, types - included
   Xbuf bcode_gen_x; // test: replacement for bcode_gen ...

   Mbuf bcode_gen;
   Mbuf bcode_parts;
   size_t bcode_len;
   Mbuf stack;
   Mbuf call_stack;
   Mbuf arg_stack;
   Mbuf scopes;          /* Scope objects */
   Mbuf loop_addresses;  /* Addresses for breaks & continues */
   Mbuf owned_strings;   /* Sequence of (varint len, char data[]) */
   Mbuf foreign_strings; /* Sequence of (varint len, char *data) */
   Mbuf owned_values;
   Mbuf json_visited_stack;
   struct mjs_vals vals;
   char *error_msg;
   char *stack_trace;
   mjs_err_t error;
   mjs_ffi_resolver_t *dlsym;  /* Symbol resolver function for FFI */
   ffi_cb_args_t *ffi_cb_args; /* List of FFI args descriptors */
   size_t cur_bcode_offset;

   gc_arena object_arena;
   gc_arena property_arena;
   gc_arena ffi_sig_arena;

   bool inhibit_gc;
   bool need_gc = false;
   unsigned generate_jsc;
};



/*
 * Tells the GC about an MJS value variable/field owned by C code.
 *
 * The user's C code should own mjs_val_t variables if the value's lifetime
 * crosses any invocation of `mjs_exec()` and friends, including `mjs_call()`.
 *
 * The registration of the variable prevents the GC from mistakenly treat the
 * object as garbage.
 *
 * User code should also explicitly disown the variables with `mjs_disown()`
 * once it goes out of scope or the structure containing the mjs_val_t field is
 * freed.
 *
 * Consider the following examples:
 *
 * Correct (owning is not necessary):
 * ```c
 * mjs_val_t res;
 * mjs_exec(mjs, "....some script", &res);
 * // ... use res somehow
 *
 * mjs_val_t res;
 * mjs_exec(mjs, "....some script2", &res);
 * // ... use new res somehow
 * ```
 *
 * WRONG:
 * ```c
 * mjs_val_t res1;
 * mjs_exec(mjs, "....some script", &res1);
 *
 * mjs_val_t res2;
 * mjs_exec(mjs, "....some script2", &res2);
 *
 * // ... use res1 (WRONG!) and res2
 * ```
 *
 * The code above is wrong, because after the second invocation of
 * `mjs_exec()`, the value of `res1` is invalidated.
 *
 * Correct (res1 is owned)
 * ```c
 * mjs_val_t res1 = MJS_UNDEFINED;
 * mjs_own(mjs, &res1);
 * mjs_exec(mjs, "....some script", &res1);
 *
 * mjs_val_t res2 = MJS_UNDEFINED;
 * mjs_exec(mjs, "....some script2", &res2);
 *
 * // ... use res1 and res2
 * mjs_disown(mjs, &res1);
 * ```
 *
 * NOTE that we explicly initialized `res1` to a valid value before owning it
 * (in this case, the value is `MJS_UNDEFINED`). Owning an uninitialized
 * variable is an undefined behaviour.
 *
 * Of course, it's not an error to own a variable even if it's not mandatory:
 * e.g. in the last example we could own both `res1` and `res2`. Probably it
 * would help us in the future, when we refactor the code so that `res2` has to
 * be owned, and we could forget to do that.
 *
 * Also, if the user code has some C function called from MJS, and in this C
 * function some MJS value (`mjs_val_t`) needs to be stored somewhhere and to
 * stay alive after the C function has returned, it also needs to be properly
 * owned.
 */
void mjs_own( Mjs *mjs, mjs_val_t *v );

/*
 * Disowns the value previously owned by `mjs_own()`.
 *
 * Returns 1 if value is found, 0 otherwise.
 */
int mjs_disown(Mjs *mjs, mjs_val_t *v);

mjs_err_t mjs_set_errorf(Mjs *mjs, mjs_err_t err, const char *fmt, ...);

/*
 * If there is no error message already set, then it's equal to
 * `mjs_set_errorf()`.
 *
 * Otherwise, an old message gets prepended with the new one, followed by a
 * colon. (the previously set error code is kept)
 */
mjs_err_t mjs_prepend_errorf( Mjs *mjs, mjs_err_t err, const char *fmt, ... );

/*
 * Print the last error details. If print_stack_trace is non-zero, also
 * print stack trace. `msg` is the message which gets prepended to the actual
 * error message, if it's NULL, then "MJS error" is used.
 */
void mjs_print_error(Mjs *mjs, FILE *fp, const char *msg, int print_stack_trace );

/*
 * return a string representation of an error.
 * the error string might be overwritten by calls to `mjs_set_errorf`.
 */
const char *mjs_strerror(Mjs *mjs, mjs_err_t err );

/*
 * Sets whether *.jsc files are generated when *.js file is executed. By
 * default it's 0.
 *
 * If either `MJS_GENERATE_JSC` or `CS_MMAP` is off, then this function has no
 * effect.
 * atu: always for me
 */
void mjs_set_generate_jsc(Mjs *mjs, int generate_jsc);

/*
 * When invoked from a cfunction, returns number of arguments passed to the
 * current JS function call.
 */
int mjs_nargs(Mjs *mjs);

/*
 * When invoked from a cfunction, returns n-th argument to the current JS
 * function call.
 */
mjs_val_t mjs_arg(Mjs *mjs, int n);

/*
 * Sets return value for the current JS function call.
 */
void mjs_return(Mjs *mjs, mjs_val_t v);



/*
 * === Arrays
 */


/* Make an empty array object */
mjs_val_t mjs_mk_array( Mjs *mjs );

/* Returns length on an array. If `arr` is not an array, 0 is returned. */
unsigned long mjs_array_length( Mjs *mjs, mjs_val_t arr );

/* Insert value `v` in array `arr` at the end of the array. */
mjs_err_t mjs_array_push( Mjs *mjs, mjs_val_t arr, mjs_val_t v );

/*
 * Return array member at index `index`. If `index` is out of bounds, undefined
 * is returned.
 */
mjs_val_t mjs_array_get( Mjs *, mjs_val_t arr, unsigned long index );

/* Insert value `v` into `arr` at index `index`. */
mjs_err_t mjs_array_set( Mjs *mjs, mjs_val_t arr, unsigned long index, mjs_val_t v );


/* Delete value in array `arr` at index `index`, if it exists. */
void mjs_array_del( Mjs *mjs, mjs_val_t arr, unsigned long index );





mjs_err_t mjs_exec( Mjs*, const char *src, mjs_val_t *res );
mjs_err_t mjs_exec_buf( Mjs*, const char *src, size_t, mjs_val_t *res );

mjs_err_t mjs_exec_file( Mjs *mjs, const char *path, mjs_val_t *res );
mjs_err_t mjs_apply( Mjs *mjs, mjs_val_t *res, mjs_val_t func, mjs_val_t this_val, int nargs, mjs_val_t *args );
mjs_err_t mjs_call( Mjs *mjs, mjs_val_t *res, mjs_val_t func, mjs_val_t this_val, int nargs, ... );
mjs_val_t mjs_get_this( Mjs *mjs );



/*
 * Returns true if the given value is an object or array.
 */
int mjs_is_object(mjs_val_t v);


/* Field types for struct-object conversion. */
enum mjs_struct_field_type {
  MJS_STRUCT_FIELD_TYPE_INVALID,
  MJS_STRUCT_FIELD_TYPE_STRUCT,     /* Struct, arg points to def. */
  MJS_STRUCT_FIELD_TYPE_STRUCT_PTR, /* Ptr to struct, arg points to def. */
  MJS_STRUCT_FIELD_TYPE_INT,
  MJS_STRUCT_FIELD_TYPE_BOOL,
  MJS_STRUCT_FIELD_TYPE_DOUBLE,
  MJS_STRUCT_FIELD_TYPE_FLOAT,
  MJS_STRUCT_FIELD_TYPE_CHAR_PTR,   /* NUL-terminated string. */
  MJS_STRUCT_FIELD_TYPE_VOID_PTR,   /* Converted to foreign ptr. */
  MJS_STRUCT_FIELD_TYPE_MG_STR_PTR, /* Converted to string. */
  MJS_STRUCT_FIELD_TYPE_MG_STR,     /* Converted to string. */
  MJS_STRUCT_FIELD_TYPE_DATA,       /* Data, arg is length, becomes string. */
  MJS_STRUCT_FIELD_TYPE_INT8,
  MJS_STRUCT_FIELD_TYPE_INT16,
  MJS_STRUCT_FIELD_TYPE_UINT8,
  MJS_STRUCT_FIELD_TYPE_UINT16,
  /*
   * User-provided function. Arg is a pointer to function that takes void *
   * (pointer to field within the struct) and returns mjs_val_t:
   * mjs_val_t field_value(Mjs *mjs, const void *field_ptr) { ... }
   */
  MJS_STRUCT_FIELD_TYPE_CUSTOM,
};

/* C structure layout descriptor - needed by mjs_struct_to_obj */
struct mjs_c_struct_member {
  const char *name;
  int offset;
  enum mjs_struct_field_type type;
  const void *arg; /* Additional argument, used for some types. */
};

/* Create flat JS object from a C memory descriptor */
mjs_val_t mjs_struct_to_obj( Mjs *mjs, const void *base, const struct mjs_c_struct_member *members );

/*
 * Lookup property `name` in object `obj`. If `obj` holds no such property,
 * an `undefined` value is returned.
 *
 * If `name_len` is ~0, `name` is assumed to be NUL-terminated and
 * `strlen(name)` is used.
 */
mjs_val_t mjs_get( Mjs *mjs, mjs_val_t obj, const char *name, size_t name_len );

/*
 * Like mjs_get but with a JS string.
 */
mjs_val_t mjs_get_v( Mjs *mjs, mjs_val_t obj, mjs_val_t name );

/*
 * Like mjs_get_v but lookup the prototype chain.
 */
mjs_val_t mjs_get_v_proto( Mjs *mjs, mjs_val_t obj, mjs_val_t key );



/*
 * Delete own property `name` of the object `obj`. Does not follow the
 * prototype chain.
 *
 * If `name_len` is ~0, `name` is assumed to be NUL-terminated and
 * `strlen(name)` is used.
 *
 * Returns 0 on success, -1 on error.
 */
int mjs_del( Mjs *mjs, mjs_val_t obj, const char *name, size_t len );

/*
 * Iterate over `obj` properties.
 * First call should set `iterator` to MJS_UNDEFINED.
 * Return object's key (a string), or MJS_UNDEFINED when no more keys left.
 * Do not mutate the object during iteration.
 *
 * Example:
 *   mjs_val_t key, iter = MJS_UNDEFINED;
 *   while ((key = mjs_next(mjs, obj, &iter)) != MJS_UNDEFINED) {
 *     // Do something with the obj/key ...
 *   }
 */
mjs_val_t mjs_next( Mjs *mjs, mjs_val_t obj, mjs_val_t *iterator );



/* Function pointer type used in `mjs_mk_foreign_func`. */
typedef void (*mjs_func_ptr_t)(void);

/*
 * Make `null` primitive value.
 *
 * NOTE: this function is deprecated and will be removed in future releases.
 * Use `MJS_NULL` instead.
 */
mjs_val_t mjs_mk_null(void);

/* Returns true if given value is a primitive `null` value */
int mjs_is_null(mjs_val_t v);

/*
 * Make `undefined` primitive value.
 *
 * NOTE: this function is deprecated and will be removed in future releases.
 * Use `MJS_UNDEFINED` instead.
 */
mjs_val_t mjs_mk_undefined(void);

/* Returns true if given value is a primitive `undefined` value */
int mjs_is_undefined(mjs_val_t v);

/* Make numeric primitive value */
mjs_val_t mjs_mk_number(Mjs *mjs, double num);

/*
 * Returns number value stored in `mjs_val_t` as `double`.
 *
 * Returns NaN for non-numbers.
 */
double mjs_get_double(Mjs *mjs, mjs_val_t v);

/*
 * Returns number value stored in `mjs_val_t` as `int`. If the number value is
 * not an integer, the fraction part will be discarded.
 *
 * If the given value is a non-number, or NaN, the result is undefined.
 */
int mjs_get_int(Mjs *mjs, mjs_val_t v);

/*
 * Like mjs_get_int but ensures that the returned type
 * is a 32-bit signed integer.
 */
int32_t mjs_get_int32(Mjs *mjs, mjs_val_t v);

/* Returns true if given value is a primitive number value */
int mjs_is_number(mjs_val_t v);

/*
 * Make JavaScript value that holds C/C++ `void *` pointer.
 *
 * A foreign value is completely opaque and JS code cannot do anything useful
 * with it except holding it in properties and passing it around.
 * It behaves like a sealed object with no properties.
 *
 * NOTE:
 * Only valid pointers (as defined by each supported architecture) will fully
 * preserved. In particular, all supported 64-bit architectures (x86_64, ARM-64)
 * actually define a 48-bit virtual address space.
 * Foreign values will be sign-extended as required, i.e creating a foreign
 * value of something like `(void *) -1` will work as expected. This is
 * important because in some 64-bit OSs (e.g. Solaris) the user stack grows
 * downwards from the end of the address space.
 *
 * If you need to store exactly sizeof(void*) bytes of raw data where
 * `sizeof(void*)` >= 8, please use byte arrays instead.
 */
mjs_val_t mjs_mk_foreign( Mjs *mjs, void *ptr );

/*
 * Make JavaScript value that holds C/C++ function pointer, similarly to
 * `mjs_mk_foreign`.
 */
mjs_val_t mjs_mk_foreign_func( Mjs *mjs, mjs_func_ptr_t fn );

/*
 * Returns `void *` pointer stored in `mjs_val_t`.
 *
 * Returns NULL if the value is not a foreign pointer.
 */
void *mjs_get_ptr(Mjs *mjs, mjs_val_t v);

/* Returns true if given value holds `void *` pointer */
int mjs_is_foreign(mjs_val_t v);

mjs_val_t mjs_mk_boolean(Mjs *mjs, int v);
int mjs_get_bool(Mjs *mjs, mjs_val_t v);
int mjs_is_boolean(mjs_val_t v);

mjs_val_t mjs_mk_function(Mjs *mjs, size_t off);
int mjs_is_function(mjs_val_t v);


#define MJS_STRING_LITERAL_MAX_LEN 128


/*
 * Creates a string primitive value.
 * `str` must point to the utf8 string of length `len`.
 * If `len` is ~0, `str` is assumed to be NUL-terminated and `strlen(str)` is
 * used.
 *
 * If `copy` is non-zero, the string data is copied and owned by the GC. The
 * caller can free the string data afterwards. Otherwise (`copy` is zero), the
 * caller owns the string data, and is responsible for not freeing it while it
 * is used.
 */
mjs_val_t mjs_mk_string( Mjs *mjs, const char *str, size_t len, int copy );

/* Returns true if given value is a primitive string value */
int mjs_is_string( mjs_val_t v );

/*
 * Returns a pointer to the string stored in `mjs_val_t`.
 *
 * String length returned in `len`, which is allowed to be NULL. Returns NULL
 * if the value is not a string.
 *
 * JS strings can contain embedded NUL chars and may or may not be NUL
 * terminated.
 *
 * CAUTION: creating new JavaScript object, array, or string may kick in a
 * garbage collector, which in turn may relocate string data and invalidate
 * pointer returned by `mjs_get_string()`.
 *
 * Short JS strings are embedded inside the `mjs_val_t` value itself. This
 * is why a pointer to a `mjs_val_t` is required. It also means that the string
 * data will become invalid once that `mjs_val_t` value goes out of scope.
 */
const char *mjs_get_string( Mjs *mjs, mjs_val_t *v, size_t *len );

/*
 * Returns a pointer to the string stored in `mjs_val_t`.
 *
 * Returns NULL if the value is not a string or if the string is not compatible
 * with a C string.
 *
 * C compatible strings contain exactly one NUL char, in terminal position.
 *
 * All strings owned by the MJS engine (see `mjs_mk_string()`) are guaranteed to
 * be NUL terminated. Out of these, those that don't include embedded NUL chars
 * are guaranteed to be C compatible.
 */
const char *mjs_get_cstring( Mjs *mjs, mjs_val_t *v );

/*
 * Returns the standard strcmp comparison code after comparing a JS string a
 * with a possibly non null-terminated string b. NOTE: the strings are equal
 * only if their length is equal, i.e. the len field doesn't imply strncmp
 * behaviour.
 */
int mjs_strcmp(Mjs *mjs, mjs_val_t *a, const char *b, size_t len);


const char *mjs_typeof(mjs_val_t v);

void mjs_fprintf(mjs_val_t v, Mjs *mjs, FILE *fp);
void mjs_sprintf(mjs_val_t v, Mjs *mjs, char *buf, size_t buflen);

void mjs_disasm(const uint8_t *code, size_t len);
void mjs_dump(Mjs *mjs, int do_disasm);

/*
 * Returns the filename corresponding to the given bcode offset.
 */
const char *mjs_get_bcode_filename_by_offset(Mjs *mjs, int offset);

/*
 * Returns the line number corresponding to the given bcode offset.
 */
int mjs_get_lineno_by_offset(Mjs *mjs, int offset);

/*
 * Returns bcode offset of the corresponding call frame cf_num, where 0 means
 * the currently executing function, 1 means the first return address, etc.
 *
 * If given cf_num is too large, -1 is returned.
 */
int mjs_get_offset_by_call_frame_num(Mjs *mjs, int cf_num);



//}; // namespace OXC_MJS
