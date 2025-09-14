#define ITEM_X 6
#define ITEM_H 16
#define ITEM_MARGIN_X 6
#define ITEM_MARGIN_Y 3
#define ITEM_W WINDOW_WIDTH - 12
#define ITEM_W_HALF (WINDOW_WIDTH - (ITEM_MARGIN_X * 3)) * 0.5
#define ITEM_W_QUARTER (WINDOW_WIDTH - (ITEM_MARGIN_X * 5)) * 0.25

#define ITEM_POS(pos) ITEM_X, 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W, ITEM_H

#define ITEM_POS_HALF_L(pos) ITEM_X, 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_HALF, ITEM_H
#define ITEM_POS_HALF_R(pos) (WINDOW_WIDTH * 0.5) + (ITEM_MARGIN_X * 0.5), 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_HALF, ITEM_H

#define ITEM_POS_QUARTER_2(pos) ITEM_X + (ITEM_MARGIN_X * 1.5) + ITEM_W_QUARTER, 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_QUARTER, ITEM_H
#define ITEM_POS_QUARTER_3(pos) (WINDOW_WIDTH * 0.5) + (ITEM_MARGIN_X * 0.5), 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_QUARTER, ITEM_H
#define ITEM_POS_QUARTER_4(pos) (WINDOW_WIDTH * 0.5) + (ITEM_MARGIN_X * 1.5) + ITEM_W_QUARTER , 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_QUARTER, ITEM_H

// labels should be offset from the side a bit as they look a bit off otherwise
// they also need to be wider to account for the potential editfield paired with them
#define LABEL_POS(pos) ITEM_X + 3, 32 + ((ITEM_H + ITEM_MARGIN_Y) * (pos - 1)), ITEM_W_HALF, ITEM_H
