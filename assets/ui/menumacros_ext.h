#define CVARINTLABEL( LABEL_X, LABEL_Y, LABEL_W, LABEL_H, LABEL_CVAR,          \
                      LABEL_TEXT_SCALE, LABEL_TEXT_ALIGN, LABEL_TEXT_ALIGN_X,  \
                      LABEL_TEXT_ALIGN_Y )                                     \
  itemDef {                                                                    \
    name            "cvarfloatlabel"##LABEL_CVAR                               \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(LABEL_X) $evalfloat(LABEL_Y)                    \
                    $evalfloat(LABEL_W) $evalfloat(LABEL_H)                    \
    type            ITEM_TYPE_TEXT                                             \
    textfont        UI_FONT_COURBD_21                                          \
    textstyle       ITEM_TEXTSTYLE_SHADOWED                                    \
    textscale       LABEL_TEXT_SCALE                                           \
    textalign       LABEL_TEXT_ALIGN                                           \
    textalignx      LABEL_TEXT_ALIGN_X                                         \
    textaligny      LABEL_TEXT_ALIGN_Y                                         \
    forecolor       .6 .6 .6 1                                                 \
    cvar            LABEL_CVAR                                                 \
    visible         1                                                          \
    decoration                                                                 \
    textasint                                                                  \
  }

#define BUTTONACTIVE( BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, BUTTON_TEXT,     \
                      BUTTON_TEXT_SCALE, BUTTON_TEXT_ALIGN_Y, BUTTON_ACTION )  \
  itemDef {                                                                    \
    name          "bttn"##BUTTON_TEXT                                          \
    group         GROUP_NAME                                                   \
    rect          $evalfloat(BUTTON_X) $evalfloat(BUTTON_Y)                    \
                  $evalfloat(BUTTON_W) $evalfloat(BUTTON_H)                    \
    type          ITEM_TYPE_BUTTON                                             \
    text          BUTTON_TEXT                                                  \
    textfont      UI_FONT_COURBD_30                                            \
    textscale     BUTTON_TEXT_SCALE                                            \
    textalign     ITEM_ALIGN_CENTER                                            \
    textalignx    $evalfloat(0.5 * BUTTON_W)                                   \
    textaligny    BUTTON_TEXT_ALIGN_Y                                          \
    style         WINDOW_STYLE_FILLED                                          \
    backcolor     .5 .5 .5 .4                                                  \
    forecolor     .9 .9 .9 1                                                   \
    border        WINDOW_BORDER_FULL                                           \
    bordercolor   .1 .1 .1 .5                                                  \
    visible       1                                                            \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor "bttn"##BUTTON_TEXT forecolor .9 .9 .9 1 ;                  \
      setitemcolor "bttn"##BUTTON_TEXT backcolor .5 .5 .5 .4                   \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor "bttn"##BUTTON_TEXT forecolor .9 .9 .9 1 ;                  \
      setitemcolor "bttn"##BUTTON_TEXT backcolor .5 .5 .5 .4                   \
    }                                                                          \
                                                                               \
    action {                                                                   \
      setitemcolor "bttn"##BUTTON_TEXT forecolor .6 .6 .6 1 ;                  \
      setitemcolor "bttn"##BUTTON_TEXT backcolor .3 .3 .3 .4 ;                 \
      play "sound/menu/select.wav" ;                                           \
      BUTTON_ACTION                                                            \
    }                                                                          \
  }

// blank rectangle with no background
#define WINDOW_BLANK( WINDOW_BLANK_X, WINDOW_BLANK_Y,                          \
                      WINDOW_BLANK_W, WINDOW_BLANK_H )                         \
  itemDef {                                                                    \
    name            "window"                                                   \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(WINDOW_BLANK_X) $evalfloat(WINDOW_BLANK_Y)      \
                    $evalfloat(WINDOW_BLANK_W) $evalfloat(WINDOW_BLANK_H)      \
    style           WINDOW_STYLE_EMPTY                                         \
    border          WINDOW_BORDER_FULL                                         \
    bordercolor     .5 .5 .5 .5                                                \
    visible         1                                                          \
    decoration                                                                 \
  }

// editfield with an adjustable textbox width
#define EDITFIELD_EXT( EDITFIELD_X, EDITFIELD_Y, EDITFIELD_W, EDITFIELD_H,     \
                       EDITFIELD_TEXTBOX_W, EDITFIELD_TEXT,                    \
                       EDITFIELD_TEXT_SCALE, EDITFIELD_TEXT_ALIGN_Y,           \
                       EDITFIELD_CVAR, EDITFIELD_MAXCHARS,                     \
                       EDITFIELD_MAXPAINTCHARS, EDITFIELD_TOOLTIP )            \
  itemDef {                                                                    \
    name            "efback"##EDITFIELD_TEXT                                   \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(EDITFIELD_X + .5 * EDITFIELD_W + 6)             \
                    $evalfloat(EDITFIELD_Y)                                    \
                    $evalfloat(.5 * EDITFIELD_TEXTBOX_W - 6)                   \
                    $evalfloat(EDITFIELD_H)                                    \
    style           WINDOW_STYLE_FILLED                                        \
    backcolor       .5 .5 .5 .2                                                \
    visible         1                                                          \
    decoration                                                                 \
  }                                                                            \
                                                                               \
  itemDef {                                                                    \
    name              "ef"##EDITFIELD_TEXT                                     \
    group             GROUP_NAME                                               \
    rect              $evalfloat(EDITFIELD_X) $evalfloat(EDITFIELD_Y)          \
                      $evalfloat(EDITFIELD_W) $evalfloat(EDITFIELD_H)          \
    type              ITEM_TYPE_EDITFIELD                                      \
    text              EDITFIELD_TEXT                                           \
    textfont          UI_FONT_COURBD_21                                        \
    textstyle         ITEM_TEXTSTYLE_SHADOWED                                  \
    textscale         EDITFIELD_TEXT_SCALE                                     \
    textalign         ITEM_ALIGN_RIGHT                                         \
    textalignx        $evalfloat(0.5 * EDITFIELD_W)                            \
    textaligny        EDITFIELD_TEXT_ALIGN_Y                                   \
    forecolor         .6 .6 .6 1                                               \
    cvar              EDITFIELD_CVAR                                           \
    maxChars          EDITFIELD_MAXCHARS                                       \
    maxPaintChars     EDITFIELD_MAXPAINTCHARS                                  \
    visible           1                                                        \
    tooltip           EDITFIELD_TOOLTIP                                        \
  }

// numericfield with an adjustable textbox width
#define NUMERICFIELD_EXT( NUMERICFIELD_X, NUMERICFIELD_Y, NUMERICFIELD_W,      \
                          NUMERICFIELD_H, NUMERICFIELD_TEXTBOX_W,              \
                          NUMERICFIELD_TEXT, NUMERICFIELD_TEXT_SCALE,          \
                          NUMERICFIELD_TEXT_ALIGN_Y, NUMERICFIELD_CVAR,        \
                          NUMERICFIELD_MAXCHARS, NUMERICFIELD_TOOLTIP )        \
  itemDef {                                                                    \
    name            "nfback"##NUMERICFIELD_TEXT                                \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(NUMERICFIELD_X + .5 * NUMERICFIELD_W + 6)       \
                    $evalfloat(NUMERICFIELD_Y)                                 \
                    $evalfloat(.5 * NUMERICFIELD_TEXTBOX_W - 6)                \
                    $evalfloat(NUMERICFIELD_H)                                 \
    style           WINDOW_STYLE_FILLED                                        \
    backcolor       .5 .5 .5 .2                                                \
    visible         1                                                          \
    decoration                                                                 \
  }                                                                            \
                                                                               \
  itemDef {                                                                    \
    name              "nf"##NUMERICFIELD_TEXT                                  \
    group             GROUP_NAME                                               \
    rect              $evalfloat(NUMERICFIELD_X) $evalfloat(NUMERICFIELD_Y)    \
                      $evalfloat(NUMERICFIELD_W) $evalfloat(NUMERICFIELD_H)    \
    type              ITEM_TYPE_NUMERICFIELD                                   \
    text              NUMERICFIELD_TEXT                                        \
    textfont          UI_FONT_COURBD_21                                        \
    textstyle         ITEM_TEXTSTYLE_SHADOWED                                  \
    textscale         NUMERICFIELD_TEXT_SCALE                                  \
    textalign         ITEM_ALIGN_RIGHT                                         \
    textalignx        $evalfloat(0.5 * NUMERICFIELD_W)                         \
    textaligny        NUMERICFIELD_TEXT_ALIGN_Y                                \
    forecolor         .6 .6 .6 1                                               \
    cvar              NUMERICFIELD_CVAR                                        \
    maxChars          NUMERICFIELD_MAXCHARS                                    \
    visible           1                                                        \
    tooltip           NUMERICFIELD_TOOLTIP                                     \
  }
