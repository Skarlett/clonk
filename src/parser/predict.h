
enum PrevisionerModeT {
  /* give list of next possible
   * tokens based on the input */
  PV_Default,

  /* follow a sequence of
   * tokens until completed */
  PV_DefSignature,
  PV_Import
};

/* Predicts the next possible tokens
 * from the current token.
 * Used to check for unexpected tokens.
 * functionality is
*/

#define PREVISION_SZ 64
union PrevisionerData {
  // default mode
  struct {
    enum onk_lexicon_t *ref;
  } default_mode;

  struct {
    uint16_t ctr;
  } fndef_mode;

  struct {
    bool has_word;
    bool expecting_junction;
  } import_mode;
};


//TODO if top of operator stack has 0 precedense, you can push ret/if/else/import
struct Previsioner {
  enum onk_lexicon_t buffer[PREVISION_SZ];
  enum PrevisionerModeT mode;
  union PrevisionerData data;
};

void init_expect_buffer(struct Previsioner *state);
