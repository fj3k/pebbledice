#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

char *modes[2] = {"Dice", "Card"};
char *rolls[4] = {"1d6", "2d6", "1d12", "1d20"};
int mode = 0;
int roll = 0;
int *cards[4];
int cardIndex[4] = {0, 0, 0, 0};

static void shuffle(int deck) {
  if (deck == -1) {
    for (int i = 0; i < 4; i++) {
      cardIndex[i] = 999;
      shuffle(i);
    }
    return;
  }
  int deckSize = 6;
  if (deck == 1) deckSize = 36;
  if (deck == 2) deckSize = 12;
  if (deck == 3) deckSize = 20;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "deck = %d, cardIndex = %d, deckSize = %d", deck, cardIndex[deck], deckSize);
  if (cardIndex[deck] < (deckSize / 3 * 2)) return;

  int sortCount = (cardIndex[deck] > deckSize) ? deckSize : cardIndex[deck];
  int sortarr[sortCount];

  //Choose cards to shuffle
  int sai = 0;
  int i = 0;
  for (; i < deckSize && sai < sortCount; i++) {
    sortarr[sai] = cards[deck][i];
    sai++;
  }

  //Move keepers to start of deck
  int nai = 0;
  for (; i < deckSize; i++) {
    cards[deck][nai] = cards[deck][i];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unshuffle deck = %d, nai = %d, card = %d", deck, (int)nai, cards[deck][nai]);
    nai++;
  }

  //Pick each remaining card at random
  for (int j = nai; j < deckSize; j++) {
    //random element
    int p = rand() % sortCount;
    while (sortarr[p] == -999) p = (p + 1) %  sortCount;
    cards[deck][nai] = sortarr[p];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "shuffle deck = %d, nai = %d, card = %d", deck, (int)nai, cards[deck][nai]);
    sortarr[p] = -999;
    nai++;
  }
  cardIndex[deck] = 0;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  roll = (roll + 1) % 4;
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, rolls[roll]);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  mode = (mode + 1) % 2;
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, modes[mode]);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  char *str = "  ";
  int val = 0;
  if (mode == 0) {
    int diceSize = 6;
    if (roll == 2) diceSize = 12;
    if (roll == 3) diceSize = 20;
    int diceCount = 1;
    if (roll == 1) diceCount = 2;
    for (int i = 0; i < diceCount; i++) {
      val += rand() % diceSize + 1;
    }
  } else {
    int deckSize = 6;
    if (roll == 1) deckSize = 36;
    if (roll == 2) deckSize = 12;
    if (roll == 3) deckSize = 20;
    cardIndex[roll] %= deckSize;
    val = cards[roll][cardIndex[roll]];
    cardIndex[roll]++; //No point normalising here, because we'd have to do it again anyway.
    shuffle(roll);
  }
  if (val >= 10)
    str[0] = '0' + (val / 10);
  else
    str[0] = ' ';
  str[1] = '0' + (val % 10);
  if (roll == 1 && val > 5 && val < 9)
    text_layer_set_text_color(text_layer, GColorDarkCandyAppleRed);
  else if (roll == 1 && val > 4 && val < 10)
    text_layer_set_text_color(text_layer, GColorBulgarianRose);
  else
    text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, str);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(GRect(0, (bounds.size.h - 50) / 2, bounds.size.w, 45));
  text_layer_set_text(text_layer, "1d6");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  cards[0] = calloc(6, sizeof(int));
  cards[1] = calloc(36, sizeof(int));
  cards[2] = calloc(12, sizeof(int));
  cards[3] = calloc(20, sizeof(int));
  int i = 0;
  for (; i < 6; i++) {
    cards[0][i] = cards[2][i] = cards[3][i] = i + 1;
    for (int j = 0; j < 6; j++) {
      cards[1][i*6+j] = (i + 1) + (j + 1);
    }
  }
  for (; i < 12; i++) {
    cards[2][i] = cards[3][i] = i + 1;
  }
  for (; i < 20; i++) {
    cards[3][i] = i + 1;
  }
  shuffle(-1);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}