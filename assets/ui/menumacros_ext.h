#define CVARINTLABEL( LABEL_X, LABEL_Y, LABEL_W, LABEL_H, LABEL_CVAR, LABEL_TEXT_SCALE, LABEL_TEXT_ALIGN, LABEL_TEXT_ALIGN_X, LABEL_TEXT_ALIGN_Y )   \
    itemDef {                                                           \
        name            "cvarfloatlabel"##LABEL_CVAR                       \
        group           GROUP_NAME                                      \
        rect            $evalfloat(LABEL_X) $evalfloat(LABEL_Y) $evalfloat(LABEL_W) $evalfloat(LABEL_H) \
        type            ITEM_TYPE_TEXT                                  \
        textfont        UI_FONT_COURBD_21                               \
        textstyle       ITEM_TEXTSTYLE_SHADOWED                         \
        textscale       LABEL_TEXT_SCALE                           \
        textalign       LABEL_TEXT_ALIGN                           \
        textalignx      LABEL_TEXT_ALIGN_X                         \
        textaligny      LABEL_TEXT_ALIGN_Y                         \
        forecolor       .6 .6 .6 1                                      \
        cvar            LABEL_CVAR                                 \
        visible         1                                               \
        decoration                                                      \
        textasint                                                     \
    }

#define BUTTONACTIVE( BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, BUTTON_TEXT, BUTTON_TEXT_SCALE, BUTTON_TEXT_ALIGN_Y, BUTTON_ACTION )                    \
    itemDef {                                                           \
        name        "bttn"##BUTTON_TEXT                                 \
        group       GROUP_NAME                                          \
        rect        $evalfloat(BUTTON_X) $evalfloat(BUTTON_Y) $evalfloat(BUTTON_W) $evalfloat(BUTTON_H)                 \
        type        ITEM_TYPE_BUTTON                                    \
        text        BUTTON_TEXT                                         \
        textfont    UI_FONT_COURBD_30                                   \
        textscale   BUTTON_TEXT_SCALE                                   \
        textalign   ITEM_ALIGN_CENTER                                   \
        textalignx  $evalfloat(0.5*(BUTTON_W))                          \
        textaligny  BUTTON_TEXT_ALIGN_Y                                 \
        style       WINDOW_STYLE_FILLED                                 \
        backcolor   .5 .5 .5 .4                                         \
        forecolor   .9 .9 .9 1                                         \
        border      WINDOW_BORDER_FULL                                  \
        bordercolor .1 .1 .1 .5                                         \
        visible     1                                                   \
                                                                        \
        mouseEnter {                                                    \
            setitemcolor "bttn"##BUTTON_TEXT forecolor .9 .9 .9 1 ;     \
            setitemcolor "bttn"##BUTTON_TEXT backcolor .5 .5 .5 .4      \
        }                                                               \
                                                                        \
        mouseExit {                                                     \
            setitemcolor "bttn"##BUTTON_TEXT forecolor .9 .9 .9 1 ;     \
            setitemcolor "bttn"##BUTTON_TEXT backcolor .5 .5 .5 .4      \
        }                                                               \
                                                                        \
        action {                                                        \
            setitemcolor "bttn"##BUTTON_TEXT forecolor .6 .6 .6 1 ;     \
            setitemcolor "bttn"##BUTTON_TEXT backcolor .3 .3 .3 .4 ;    \
            play "sound/menu/select.wav" ;                              \
            BUTTON_ACTION                                               \
        }                                                               \
    }
    