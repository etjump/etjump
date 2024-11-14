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

#define NAMEDBUTTONACTIVE( BUTTON_NAME, BUTTON_X, BUTTON_Y, BUTTON_W,          \
                           BUTTON_H, BUTTON_TEXT, BUTTON_TEXT_SCALE,           \
                           BUTTON_TEXT_ALIGN_Y, BUTTON_ACTION )                \
  itemDef {                                                                    \
    name          BUTTON_NAME                                                  \
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

// dropdown menu
#define COMBO( COMBO_X, COMBO_Y, COMBO_W, COMBO_H, COMBO_RECT_W,               \
               COMBO_MAX_ITEMS, COMBO_REVERSED, COMBO_TEXT, COMBO_TEXT_SCALE,  \
               COMBO_TEXT_ALIGN_Y, COMBO_CVAR, COMBO_CVARLIST,                 \
               COMBO_ACTION, COMBO_TOOLTIP )                                   \
                                                                               \
  itemDef {                                                                    \
    name            "comboback"##COMBO_TEXT                                    \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(COMBO_X + 0.5 * COMBO_W + 6)                    \
                    $evalfloat(COMBO_Y)                                        \
                    $evalfloat(0.5 * COMBO_RECT_W - 6)                         \
                    $evalfloat(COMBO_H)                                        \
    style           WINDOW_STYLE_FILLED                                        \
    backcolor       .3 .3 .3 .4                                                \
    visible         1                                                          \
    decoration                                                                 \
  }                                                                            \
                                                                               \
  itemDef {                                                                    \
    name              "combo"##COMBO_TEXT                                      \
    group             GROUP_NAME                                               \
    rect              $evalfloat(COMBO_X) $evalfloat(COMBO_Y)                  \
                      $evalfloat(COMBO_W) $evalfloat(COMBO_H)                  \
    comboRect         $evalfloat(COMBO_X) $evalfloat(COMBO_Y)                  \
                      $evalfloat(COMBO_RECT_W) $evalfloat(COMBO_H)             \
    type              ITEM_TYPE_COMBO                                          \
    text              COMBO_TEXT                                               \
    textfont          UI_FONT_COURBD_21                                        \
    textstyle         ITEM_TEXTSTYLE_SHADOWED                                  \
    textscale         COMBO_TEXT_SCALE                                         \
    textalign         ITEM_ALIGN_RIGHT                                         \
    textalignx        $evalfloat(0.5 * (COMBO_W))                              \
    textaligny        COMBO_TEXT_ALIGN_Y                                       \
    backcolor         .3 .3 .3 .4                                              \
    forecolor         .6 .6 .6 1                                               \
    cvar              COMBO_CVAR                                               \
    COMBO_CVARLIST                                                             \
    visible           1                                                        \
    bordercolor       .1 .1 .1 .5                                              \
    tooltip           COMBO_TOOLTIP                                            \
    combomaxitems     $evalfloat(COMBO_MAX_ITEMS)                              \
    comboreversed     COMBO_REVERSED                                           \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor "combo"##COMBO_TEXT forecolor .9 .9 .9 1 ;                  \
      setitemcolor "combo"##COMBO_TEXT backcolor .5 .5 .5 .4 ;                 \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor "combo"##COMBO_TEXT forecolor .6 .6 .6 1 ;                  \
      setitemcolor "combo"##COMBO_TEXT backcolor .3 .3 .3 .4 ;                 \
    }                                                                          \
                                                                               \
    accept {                                                                   \
      play "sound/menu/filter.wav" ;                                           \
      COMBO_ACTION                                                             \
    }                                                                          \
                                                                               \
    action {                                                                   \
      play "sound/menu/filter.wav" ;                                           \
    }                                                                          \
  }

// dropdown menu with bitflag selection
#define COMBO_BIT( COMBO_BIT_X, COMBO_BIT_Y, COMBO_BIT_W, COMBO_BIT_H,         \
                   COMBO_BIT_RECT_W, COMBO_BIT_MAX_ITEMS, COMBO_BIT_REVERSED,  \
                   COMBO_BIT_TEXT, COMBO_BIT_TEXT_SCALE,                       \
                   COMBO_BIT_TEXT_ALIGN_Y, COMBO_BIT_CVAR, COMBO_BIT_CVARLIST, \
                   COMBO_BIT_ACTION, COMBO_BIT_TOOLTIP )                       \
                                                                               \
  itemDef {                                                                    \
    name            "comboback"##COMBO_BIT_TEXT                                \
    group           GROUP_NAME                                                 \
    rect            $evalfloat(COMBO_BIT_X + (0.5 * COMBO_BIT_W) + 6)          \
                    $evalfloat(COMBO_BIT_Y)                                    \
                    $evalfloat(0.5 * COMBO_BIT_RECT_W - 6)                     \
                    $evalfloat(COMBO_BIT_H)                                    \
    style           WINDOW_STYLE_FILLED                                        \
    backcolor       .3 .3 .3 .4                                                \
    visible         1                                                          \
    decoration                                                                 \
  }                                                                            \
                                                                               \
  itemDef {                                                                    \
    name              "combo"##COMBO_BIT_TEXT                                  \
    group             GROUP_NAME                                               \
    rect              $evalfloat(COMBO_BIT_X) $evalfloat(COMBO_BIT_Y)          \
                      $evalfloat(COMBO_BIT_W) $evalfloat(COMBO_BIT_H)          \
    comboRect         $evalfloat(COMBO_BIT_X) $evalfloat(COMBO_BIT_Y)          \
                      $evalfloat(COMBO_BIT_RECT_W) $evalfloat(COMBO_BIT_H)     \
    type              ITEM_TYPE_COMBO                                          \
    text              COMBO_BIT_TEXT                                           \
    textfont          UI_FONT_COURBD_21                                        \
    textstyle         ITEM_TEXTSTYLE_SHADOWED                                  \
    textscale         COMBO_BIT_TEXT_SCALE                                     \
    textalign         ITEM_ALIGN_RIGHT                                         \
    textalignx        $evalfloat(0.5 * (COMBO_BIT_W))                          \
    textaligny        COMBO_BIT_TEXT_ALIGN_Y                                   \
    backcolor         .3 .3 .3 .4                                              \
    forecolor         .6 .6 .6 1                                               \
    cvar              COMBO_BIT_CVAR                                           \
    COMBO_BIT_CVARLIST                                                         \
    visible           1                                                        \
    bordercolor       .1 .1 .1 .5                                              \
    tooltip           COMBO_BIT_TOOLTIP                                        \
    combomaxitems     $evalfloat(COMBO_BIT_MAX_ITEMS)                          \
    combobitflag                                                               \
    comboreversed     COMBO_BIT_REVERSED                                       \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor "combo"##COMBO_BIT_TEXT forecolor .9 .9 .9 1 ;              \
      setitemcolor "combo"##COMBO_BIT_TEXT backcolor .5 .5 .5 .4 ;             \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor "combo"##COMBO_BIT_TEXT forecolor .6 .6 .6 1 ;              \
      setitemcolor "combo"##COMBO_BIT_TEXT backcolor .3 .3 .3 .4 ;             \
    }                                                                          \
                                                                               \
    accept {                                                                   \
      play "sound/menu/filter.wav" ;                                           \
      COMBO_BIT_ACTION                                                         \
    }                                                                          \
                                                                               \
    action {                                                                   \
      play "sound/menu/filter.wav" ;                                           \
    }                                                                          \
  }

// a slider that adjusts HSV/RGB/A values, used in color picker
#define SLIDER_COLOR( SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, SLIDER_TEXT,     \
                      SLIDER_TEXT_SCALE, SLIDER_TEXT_ALIGN_Y,                  \
                      SLIDER_COLORVAR, SLIDER_TOOLTIP )                        \
                                                                               \
  itemDef {                                                                    \
    name          "sliderColor"##SLIDER_TEXT                                   \
    group         GROUP_NAME                                                   \
    rect          $evalfloat(SLIDER_X) $evalfloat(SLIDER_Y)                    \
                  $evalfloat(SLIDER_W) $evalfloat(SLIDER_H)                    \
    type          ITEM_TYPE_SLIDER                                             \
    text          SLIDER_TEXT                                                  \
    textfont      UI_FONT_COURBD_21                                            \
    textstyle     ITEM_TEXTSTYLE_SHADOWED                                      \
    textscale     SLIDER_TEXT_SCALE                                            \
    textalign     ITEM_ALIGN_RIGHT                                             \
    textalignx    $evalfloat(0.5*(SLIDER_W))                                   \
    textaligny    SLIDER_TEXT_ALIGN_Y                                          \
    forecolor     .6 .6 .6 1                                                   \
    colorVar      SLIDER_COLORVAR                                              \
    visible       1                                                            \
    tooltip       SLIDER_TOOLTIP                                               \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor "sliderColor"##SLIDER_TEXT forecolor .9 .9 .9 1 ;           \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor "sliderColor"##SLIDER_TEXT forecolor .6 .6 .6 1 ;           \
    }                                                                          \
  }

// displays the color value set by SLIDER_COLOR as int
#define SLIDERCOLORINTLABEL( LABEL_X, LABEL_Y, LABEL_W, LABEL_H,               \
                             LABEL_COLORVAR, LABEL_TEXT_SCALE,                 \
                             LABEL_TEXT_ALIGN, LABEL_TEXT_ALIGN_X,             \
                             LABEL_TEXT_ALIGN_Y )                              \
                                                                               \
  itemDef {                                                                    \
    name            "sliderLabel"##LABEL_COLORVAR                              \
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
    colorValue      LABEL_COLORVAR                                             \
    visible         1                                                          \
    decoration                                                                 \
    textasint                                                                  \
  }

// displays the color value set by SLIDER_COLOR as float
#define SLIDERCOLORFLOATLABEL( LABEL_X, LABEL_Y, LABEL_W, LABEL_H,             \
                               LABEL_COLORVAR, LABEL_TEXT_SCALE,               \
                               LABEL_TEXT_ALIGN, LABEL_TEXT_ALIGN_X,           \
                               LABEL_TEXT_ALIGN_Y )                            \
                                                                               \
  itemDef {                                                                    \
    name            "sliderLabel"##LABEL_COLORVAR                              \
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
    colorValue      LABEL_COLORVAR                                             \
    visible         1                                                          \
    decoration                                                                 \
    textasfloat                                                                \
  }

// note: this is NOT a decoration so we can grab focus!
#define COLORPICKER( COLORPICKER_X, COLORPICKER_Y, COLORPICKER_W,              \
                     COLORPICKER_H, COLORPICKER_NAME, COLORPICKER_OWNERNUM,    \
                     COLORPICKER_TOOLTIP )                                     \
                                                                               \
  itemDef {                                                                    \
    name          COLORPICKER_NAME                                             \
    group         GROUP_NAME                                                   \
    rect          $evalfloat(COLORPICKER_X) $evalfloat(COLORPICKER_Y)          \
                  $evalfloat(COLORPICKER_W) $evalfloat(COLORPICKER_H)          \
    type          ITEM_TYPE_OWNERDRAW                                          \
    ownerdraw     COLORPICKER_OWNERNUM                                         \
    textfont      UI_FONT_COURBD_21                                            \
    textscale     .2                                                           \
    border        WINDOW_BORDER_FULL                                           \
    bordercolor   .5 .5 .5 .5                                                  \
    tooltip       COLORPICKER_TOOLTIP                                          \
    tooltipAbove                                                               \
    visible       1                                                            \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor COLORPICKER_NAME bordercolor .75 .75 .75 .5                 \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor COLORPICKER_NAME bordercolor .5 .5 .5 .5                    \
    }                                                                          \
  }


#define COLORPICKER_PREVIEW( OCOLORPICKER_PREVIEW_X, COLORPICKER_PREVIEW_Y,    \
                             COLORPICKER_PREVIEW_W, COLORPICKER_PREVIEW_H,     \
                             COLORPICKER_PREVIEW_NAME,                         \
                             COLORPICKER_PREVIEW_OWNERNUM,                     \
                             COLORPICKER_PREVIEW_TOOLTIP )                     \
                                                                               \
  itemDef {                                                                    \
    name          COLORPICKER_PREVIEW_NAME                                     \
    group         GROUP_NAME                                                   \
    rect          $evalfloat(OCOLORPICKER_PREVIEW_X)                           \
                  $evalfloat(COLORPICKER_PREVIEW_Y)                            \
                  $evalfloat(COLORPICKER_PREVIEW_W)                            \
                  $evalfloat(COLORPICKER_PREVIEW_H)                            \
    type          ITEM_TYPE_OWNERDRAW                                          \
    ownerdraw     COLORPICKER_PREVIEW_OWNERNUM                                 \
    textfont      UI_FONT_COURBD_21                                            \
    textscale     .2                                                           \
    border        WINDOW_BORDER_FULL                                           \
    bordercolor   .5 .5 .5 .5                                                  \
    tooltip       COLORPICKER_PREVIEW_TOOLTIP                                  \
    visible       1                                                            \
    decoration                                                                 \
  }

// slider that only updates the cvar when mouse button is released
#define CACHEDSLIDER( CACHEDSLIDER_X, CACHEDSLIDER_Y, CACHEDSLIDER_W,          \
                      CACHEDSLIDER_H, CACHEDSLIDER_TEXT,                       \
                      CACHEDSLIDER_TEXT_SCALE, CACHEDSLIDER_TEXT_ALIGN_Y,      \
                      CACHEDSLIDER_CVARFLOAT, CACHEDSLIDER_TOOLTIP )           \
  itemDef {                                                                    \
    name        "cachedslider"##CACHEDSLIDER_TEXT                              \
    group       GROUP_NAME                                                     \
    rect        $evalfloat(CACHEDSLIDER_X) $evalfloat(CACHEDSLIDER_Y)          \
                $evalfloat(CACHEDSLIDER_W) $evalfloat(CACHEDSLIDER_H)          \
    type        ITEM_TYPE_SLIDER                                               \
    text        CACHEDSLIDER_TEXT                                              \
    textfont    UI_FONT_COURBD_21                                              \
    textstyle   ITEM_TEXTSTYLE_SHADOWED                                        \
    textscale   CACHEDSLIDER_TEXT_SCALE                                        \
    textalign   ITEM_ALIGN_RIGHT                                               \
    textalignx  $evalfloat(0.5*(CACHEDSLIDER_W))                               \
    textaligny  CACHEDSLIDER_TEXT_ALIGN_Y                                      \
    forecolor   .6 .6 .6 1                                                     \
    cvarFloat   CACHEDSLIDER_CVARFLOAT                                         \
    visible     1                                                              \
    tooltip     CACHEDSLIDER_TOOLTIP                                           \
    cacheCvar                                                                  \
                                                                               \
    mouseEnter {                                                               \
      setitemcolor "cachedslider"##CACHEDSLIDER_TEXT forecolor .9 .9 .9 1 ;    \
    }                                                                          \
                                                                               \
    mouseExit {                                                                \
      setitemcolor "cachedslider"##CACHEDSLIDER_TEXT forecolor .6 .6 .6 1 ;    \
    }                                                                          \
  }
