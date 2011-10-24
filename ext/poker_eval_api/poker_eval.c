#include "ruby.h"

/*
 *
 * Corbright (C) 2007, 2008 Loic Dachary <loic@dachary.org>
 * Corbright (C) 2004, 2005, 2006 Mekensleep
 *
 *	Mekensleep
 *	24 rue vieille du temple
 *	75004 Paris
 *       licensing@mekensleep.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a corb of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Authors:
 *  Loic Dachary <loic@dachary.org>
 *
 */


/* enumerate.c -- functions to compute pot equity by enumerating outcomes
  Exports:
        enumExhaustive()	exhaustive enumeration of outcomes
        enumGameParams()	look up rule parameters by game type
        enumResultClear()	clear enumeration result object
        enumResultPrint()	print enumeration result object
        enumResultPrintTerse()	print enumeration result object, tersely
        enumSample()		monte carlo sampling of outcomes

   Michael Maurer, Apr 2002
*/

#include "poker_defs.h"
#include "inlines/eval.h"
#include "inlines/eval_low.h"
#include "inlines/eval_low8.h"
#include "inlines/eval_joker_low.h"
#include "inlines/eval_omaha.h"
#include "deck_std.h"
#include "rules_std.h"

#include "enumerate.h"
#include "enumdefs.h"

/* INNER_LOOP is executed in every iteration of the combinatorial enumerator
   macros DECK_ENUMERATE_n_CARDS_D() and DECK_ENUMERATE_PERMUTATIONS_D.  It
   evaluates each player's hand based on the enumerated community cards and
   accumulates statistics on wins, ties, losses, and pot equity.

   Macro argument:
   	evalwrap -- code that evaluates pockets[i], board, sharedCards, and/or
        	    unsharedCards[i] as a poker hand, then stores the result
                    in hival[i] and loval[i] and stores an error code in err
   Loop variable: either of
   	StdDeck_CardMask sharedCards;
   	StdDeck_CardMask unsharedCards[];
   Inputs:
   	StdDeck_CardMask pockets[];
        StdDeck_CardMask board;
        int npockets;
   Outputs:
   	enum_result_t *result;
*/

#define INNER_LOOP(evalwrap)						\
    do {								\
      int i;								\
      HandVal hival[ENUM_MAXPLAYERS];					\
      LowHandVal loval[ENUM_MAXPLAYERS];				\
      HandVal besthi = HandVal_NOTHING;					\
      LowHandVal bestlo = LowHandVal_NOTHING;				\
      int hishare = 0;							\
      int loshare = 0;							\
      double hipot, lopot;						\
      /* find winning hands for high and low */				\
      for (i=0; i<sizeToDeal-1; i++) {					\
	int err;							\
        { evalwrap }							\
        if (err != 0)							\
          return 1000 + err;						\
        if (hival[i] != HandVal_NOTHING) {				\
          if (hival[i] > besthi) {					\
            besthi = hival[i];						\
            hishare = 1;						\
          } else if (hival[i] == besthi) {				\
            hishare++;							\
          }								\
        }								\
        if (loval[i] != LowHandVal_NOTHING) {				\
          if (loval[i] < bestlo) {					\
            bestlo = loval[i];						\
            loshare = 1;						\
          } else if (loval[i] == bestlo) {				\
            loshare++;							\
          }								\
        }								\
      }									\
      /* now award pot fractions to winning hands */			\
      if (bestlo != LowHandVal_NOTHING &&				\
          besthi != HandVal_NOTHING) {					\
        hipot = 0.5 / hishare;						\
        lopot = 0.5 / loshare;						\
      } else if (bestlo == LowHandVal_NOTHING &&			\
                 besthi != HandVal_NOTHING) {				\
        hipot = 1.0 / hishare;						\
        lopot = 0;							\
      } else if (bestlo != LowHandVal_NOTHING &&			\
                 besthi == HandVal_NOTHING) {				\
        hipot = 0;							\
        lopot = 1.0 / loshare;						\
      } else {								\
        hipot = lopot = 0;						\
      }									\
      for (i=0; i<sizeToDeal-1; i++) {					\
        double potfrac = 0;						\
        int H = 0, L = 0;						\
        if (hival[i] != HandVal_NOTHING) {				\
          if (hival[i] == besthi) {					\
            H = hishare;						\
            potfrac += hipot;						\
            if (hishare == 1)						\
              result->nwinhi[i]++;					\
             else							\
              result->ntiehi[i]++;					\
          } else {							\
            result->nlosehi[i]++;					\
          }								\
        }								\
        if (loval[i] != LowHandVal_NOTHING) {				\
          if (loval[i] == bestlo) {					\
            L = loshare;						\
            potfrac += lopot;						\
            if (loshare == 1)						\
              result->nwinlo[i]++;					\
            else							\
              result->ntielo[i]++;					\
          } else {							\
            result->nloselo[i]++;					\
          }								\
        }								\
        result->nsharehi[i][H]++;					\
        result->nsharelo[i][L]++;					\
        result->nshare[i][H][L]++;					\
        if (potfrac > 0.99)						\
          result->nscoop[i]++;						\
        result->ev[i] += potfrac;					\
      }									\
      result->nsamples++;						\
    } while (0);

#define INNER_LOOP_ANY_HIGH						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask _finalBoard;					\
    StdDeck_CardMask_RESET(_hand);					\
    StdDeck_CardMask_RESET(_finalBoard);				\
    StdDeck_CardMask_OR(_finalBoard, board, cardsDealt[0]);		\
    StdDeck_CardMask_OR(_hand, pockets[i], _finalBoard);		\
    StdDeck_CardMask_OR(_hand, _hand, cardsDealt[i + 1]);		\
    hival[i] = StdDeck_StdRules_EVAL_N(_hand, 7);			\
    loval[i] = LowHandVal_NOTHING;					\
    err = 0;								\
  })

#define INNER_LOOP_ANY_HILO						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask _finalBoard;					\
    StdDeck_CardMask_RESET(_hand);					\
    StdDeck_CardMask_RESET(_finalBoard);				\
    StdDeck_CardMask_OR(_finalBoard, board, cardsDealt[0]);		\
    StdDeck_CardMask_OR(_hand, pockets[i], _finalBoard);		\
    StdDeck_CardMask_OR(_hand, _hand, cardsDealt[i + 1]);		\
    hival[i] = StdDeck_StdRules_EVAL_N(_hand, 7);			\
    loval[i] = StdDeck_Lowball8_EVAL(_hand, 7);				\
    err = 0;								\
  })

#define INNER_LOOP_OMAHA						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask _finalBoard;					\
    StdDeck_CardMask_RESET(_hand);					\
    StdDeck_CardMask_RESET(_finalBoard);				\
    StdDeck_CardMask_OR(_finalBoard, board, cardsDealt[0]);		\
    StdDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);		\
    err = StdDeck_OmahaHiLow8_EVAL(_hand, _finalBoard,                  \
                                   &hival[i], NULL);			\
    loval[i] = LowHandVal_NOTHING;					\
  })

#define INNER_LOOP_OMAHA8						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask _finalBoard;					\
    StdDeck_CardMask_RESET(_hand);					\
    StdDeck_CardMask_RESET(_finalBoard);				\
    StdDeck_CardMask_OR(_finalBoard, board, cardsDealt[0]);		\
    StdDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);		\
    err = StdDeck_OmahaHiLow8_EVAL(_hand, _finalBoard,                  \
                                   &hival[i], &loval[i]);		\
  })

#define INNER_LOOP_7STUDNSQ						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);		\
    hival[i] = StdDeck_StdRules_EVAL_N(_hand, 7);			\
    loval[i] = StdDeck_Lowball_EVAL(_hand, 7);				\
    err = 0;								\
  })

#define INNER_LOOP_RAZZ							\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);		\
    hival[i] = HandVal_NOTHING;						\
    loval[i] = StdDeck_Lowball_EVAL(_hand, 7);				\
    err = 0;								\
  })

#define INNER_LOOP_5DRAW						\
  INNER_LOOP({								\
    JokerDeck_CardMask _hand;						\
    JokerDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);	\
    hival[i] = JokerDeck_JokerRules_EVAL_N(_hand, 5);			\
    loval[i] = LowHandVal_NOTHING;					\
    err = 0;								\
  })

#define INNER_LOOP_5DRAW8						\
  INNER_LOOP({								\
    JokerDeck_CardMask _hand;						\
    JokerDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);	\
    hival[i] = JokerDeck_JokerRules_EVAL_N(_hand, 5);			\
    loval[i] = JokerDeck_Lowball8_EVAL(_hand, 5);			\
    err = 0;								\
  })

#define INNER_LOOP_5DRAWNSQ						\
  INNER_LOOP({								\
    JokerDeck_CardMask _hand;						\
    JokerDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);	\
    hival[i] = JokerDeck_JokerRules_EVAL_N(_hand, 5);			\
    loval[i] = JokerDeck_Lowball_EVAL(_hand, 5);			\
    err = 0;								\
  })

#define INNER_LOOP_LOWBALL						\
  INNER_LOOP({								\
    JokerDeck_CardMask _hand;						\
    JokerDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);	\
    hival[i] = HandVal_NOTHING;						\
    loval[i] = JokerDeck_Lowball_EVAL(_hand, 5);			\
    err = 0;								\
  })

#define INNER_LOOP_LOWBALL27						\
  INNER_LOOP({								\
    StdDeck_CardMask _hand;						\
    StdDeck_CardMask_OR(_hand, pockets[i], cardsDealt[i + 1]);		\
    hival[i] = HandVal_NOTHING;						\
    loval[i] = StdDeck_StdRules_EVAL_N(_hand, 5);			\
    err = 0;								\
  })

static int 
rbenumExhaustive(enum_game_t game, StdDeck_CardMask pockets[],
		 int numToDeal[],
               StdDeck_CardMask board, StdDeck_CardMask dead,
               int sizeToDeal, enum_result_t *result) {
  int totalToDeal = 0;
  int i;
  enumResultClear(result);
  StdDeck_CardMask cardsDealt[ENUM_MAXPLAYERS + 1];
  memset(cardsDealt, 0, sizeof(StdDeck_CardMask) * (ENUM_MAXPLAYERS + 1));
  if (sizeToDeal - 1 > ENUM_MAXPLAYERS)
    return 1;
  for(i = 0; i < sizeToDeal; i++)
    totalToDeal += numToDeal[i];

  /*
   * Cards in pockets or in the board must not be dealt 
   */
  StdDeck_CardMask_OR(dead, dead, board);
  for(i = 0; i < sizeToDeal - 1; i++) {
    StdDeck_CardMask_OR(dead, dead, pockets[i]);
  }

  if (game == game_holdem) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_ANY_HIGH);
    } else {
      INNER_LOOP_ANY_HIGH;
    }
  } else if (game == game_holdem8) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_ANY_HILO);
    } else {
      INNER_LOOP_ANY_HILO;
    }
  } else if (game == game_omaha) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_OMAHA);
    } else {
      INNER_LOOP_OMAHA;
    }
  } else if (game == game_omaha8) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_OMAHA8);
    } else {
      INNER_LOOP_OMAHA8;
    }
  } else if (game == game_7stud) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_ANY_HIGH);
    } else {
      INNER_LOOP_ANY_HIGH;
    }
  } else if (game == game_7stud8) {
    if(totalToDeal > 0) {
      DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
				    sizeToDeal, numToDeal,
				    dead, INNER_LOOP_ANY_HILO);
    } else {
      INNER_LOOP_ANY_HILO;
    }
  } else if (game == game_7studnsq) {
    DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
                                  sizeToDeal, numToDeal,
                                  dead, INNER_LOOP_7STUDNSQ);
  } else if (game == game_razz) {
    DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
                                  sizeToDeal, numToDeal,
                                  dead, INNER_LOOP_RAZZ);
  } else if (game == game_lowball27) {
    DECK_ENUMERATE_COMBINATIONS_D(StdDeck, cardsDealt,
                                  sizeToDeal, numToDeal,
                                  dead, INNER_LOOP_LOWBALL27);
  } else {
    return 1;
  }

  result->game = game;
  result->nplayers = sizeToDeal - 1;
  result->sampleType = ENUM_EXHAUSTIVE;
  return 0;  
}

static int 
rbenumSample(enum_game_t game, StdDeck_CardMask pockets[],
		 int numToDeal[],
               StdDeck_CardMask board, StdDeck_CardMask dead,
               int sizeToDeal, int iterations, enum_result_t *result) {
  int i;
  enumResultClear(result);
  StdDeck_CardMask cardsDealt[ENUM_MAXPLAYERS + 1];
  memset(cardsDealt, 0, sizeof(StdDeck_CardMask) * (ENUM_MAXPLAYERS + 1));
  if (sizeToDeal - 1 > ENUM_MAXPLAYERS)
    return 1;

  /*
   * Cards in pockets or in the board must not be dealt 
   */
  StdDeck_CardMask_OR(dead, dead, board);
  for(i = 0; i < sizeToDeal - 1; i++) {
    StdDeck_CardMask_OR(dead, dead, pockets[i]);
  }

  if (game == game_holdem) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_ANY_HIGH);
  } else if (game == game_holdem8) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_ANY_HILO);
  } else if (game == game_omaha) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_OMAHA);
  } else if (game == game_omaha8) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_OMAHA8);
  } else if (game == game_7stud) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_ANY_HIGH);
  } else if (game == game_7stud8) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_ANY_HILO);
  } else if (game == game_7studnsq) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_7STUDNSQ);
  } else if (game == game_razz) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_RAZZ);
  } else if (game == game_lowball27) {
    DECK_MONTECARLO_PERMUTATIONS_D(StdDeck, cardsDealt,
				   sizeToDeal, numToDeal,
				   dead, iterations, INNER_LOOP_LOWBALL27);
  } else {
    return 1;
  }

  result->game = game;
  result->nplayers = sizeToDeal - 1;
  result->sampleType = ENUM_SAMPLE;
  return 0;  
}

#define NOCARD 255

static int rbList2CardMask(VALUE object, CardMask* cardsp)
{
  CardMask cards;
  int cards_size = 0;
  int valid_cards_size = 0;

  if (TYPE(object) != T_ARRAY)
  {
    rb_fatal("expected a list of cards");
  }

  valid_cards_size = cards_size = RARRAY_LENINT(object);
  CardMask_RESET(cards);

  int card;
  int i;
  for(i = 0; i < cards_size; i++) {
    card = -1;
    char* card_string = RSTRING_PTR(rb_ary_entry(object, i));

    if(!strcmp(card_string, "__"))
        card = 255;
    else
        if(Deck_stringToCard(card_string, &card) == 0)
            rb_fatal("card %s is not a valid card name", card_string);

    if(card == NOCARD)
      valid_cards_size--;
    else
      CardMask_SET(cards, card);
  }

  *cardsp = cards;

  return valid_cards_size;
}

static VALUE
t_eval(VALUE self, VALUE args)
{
  int i;
  int pockets_size;
  int iterations = 0;
  VALUE rbpockets = 0;
  VALUE rbboard = 0;
  char* game = 0;
  VALUE rbdead = 0;
  enum_gameparams_t* params = 0;


  game = RSTRING_PTR(rb_hash_aref(args, rb_str_new2("game")));
  rbpockets = rb_hash_aref(args, rb_str_new2("pockets"));
  rbboard = rb_hash_aref(args, rb_str_new2("board"));
  rbdead = rb_hash_aref(args, rb_str_new2("dead"));
  iterations = FIX2INT(rb_hash_aref(args, rb_str_new2("iterations")));


  StdDeck_CardMask pockets[ENUM_MAXPLAYERS];
  int numToDeal[ENUM_MAXPLAYERS];
  CardMask dead_cards;
  CardMask board_cards;

  VALUE result = 0;

  if(!strcmp(game, "holdem")) {
    params = enumGameParams(game_holdem);
  } else if(!strcmp(game, "holdem8")) {
    params = enumGameParams(game_holdem8);
  } else if(!strcmp(game, "omaha")) {
    params = enumGameParams(game_omaha);
  } else if(!strcmp(game, "omaha8")) {
    params = enumGameParams(game_omaha8);
  } else if(!strcmp(game, "7stud")) {
    params = enumGameParams(game_7stud);
  } else if(!strcmp(game, "7stud8")) {
    params = enumGameParams(game_7stud8);
  } else if(!strcmp(game, "7studnsq")) {
    params = enumGameParams(game_7studnsq);
  } else if(!strcmp(game, "razz")) {
    params = enumGameParams(game_razz);
  } else if(!strcmp(game, "5draw")) {
    params = enumGameParams(game_5draw);
  } else if(!strcmp(game, "5draw8")) {
    params = enumGameParams(game_5draw8);
  } else if(!strcmp(game, "5drawnsq")) {
    params = enumGameParams(game_5drawnsq);
  } else if(!strcmp(game, "lowball")) {
    params = enumGameParams(game_lowball);
  } else if(!strcmp(game, "lowball27")) {
    params = enumGameParams(game_lowball27);
  }

  if(params == 0)
    rb_fatal("game %s is not a valid value (holdem, holdem8, omaha, omaha8, 7stud, 7stud8, 7studnsq, razz, 5draw, 5draw8, 5drawnsq, lowball, lowball27)", game);

  if (TYPE(rbpockets) != T_ARRAY)
    rb_fatal("pockets must be list");

  pockets_size = RARRAY_LENINT(rbpockets);

  {
    for(i = 0; i < pockets_size; i++) {
      int count;
      CardMask_RESET(pockets[i]);
      VALUE rbpocket = rb_ary_entry(rbpockets, i);

      count = rbList2CardMask(rbpocket, &pockets[i]);

      if(count < 0)
        goto err;
      if(count < RARRAY_LEN(rbpocket))
        numToDeal[i + 1] = RARRAY_LENINT(rbpocket) - count;
      else
        numToDeal[i + 1] = 0;
    }
  }


  {
    int count;
    count = rbList2CardMask(rbboard, &board_cards);
    if(count < 0)
      goto err;
    if(count < RARRAY_LENINT(rbboard))
      numToDeal[0] = RARRAY_LENINT(rbboard) - count;
    else
      numToDeal[0] = 0;
  }

  if(!NIL_P(rbdead) && RARRAY_LEN(rbdead) > 0) {
    if(rbList2CardMask(rbdead, &dead_cards) < 0){
      rb_fatal("dead cards error");
    }
  }
  else {
      CardMask_RESET(dead_cards);
  }

  {
    enum_result_t cresult;
    int err;
    memset(&cresult, '\0', sizeof(enum_result_t));

    if(iterations > 0) {
      err = rbenumSample(params->game, pockets, numToDeal, board_cards, dead_cards, pockets_size + 1, iterations, &cresult);
    } else {
      err = rbenumExhaustive(params->game, pockets, numToDeal, board_cards, dead_cards, pockets_size + 1, &cresult);
    }
    if(err != 0) {
      rb_fatal("poker-eval: rbenum returned error code %d", err);
    }

    result = rb_hash_new();

    VALUE info = rb_hash_new(); 
    rb_hash_aset(info, rb_str_new2("samples"), INT2NUM(cresult.nsamples));
    rb_hash_aset(info, rb_str_new2("haslopot"), INT2NUM(params->haslopot));
    rb_hash_aset(info, rb_str_new2("hashipot"), INT2NUM(params->hashipot));

    rb_hash_aset(result, rb_str_new2("info"), info);

    VALUE list = rb_ary_new();
    for(i = 0; i < pockets_size; i++) {
      VALUE tmp = rb_hash_new(); 
      rb_hash_aset(tmp, rb_str_new2("scoop"), INT2NUM(cresult.nscoop[i]));
      rb_hash_aset(tmp, rb_str_new2("winhi"), INT2NUM(cresult.nwinhi[i]));
      rb_hash_aset(tmp, rb_str_new2("losehi"), INT2NUM(cresult.nlosehi[i]));
      rb_hash_aset(tmp, rb_str_new2("tiehi"), INT2NUM(cresult.ntiehi[i]));
      rb_hash_aset(tmp, rb_str_new2("winlo"), INT2NUM(cresult.nwinlo[i]));
      rb_hash_aset(tmp, rb_str_new2("loselo"), INT2NUM(cresult.nloselo[i]));
      rb_hash_aset(tmp, rb_str_new2("tielo"), INT2NUM(cresult.ntielo[i]));
      rb_hash_aset(tmp, rb_str_new2("ev"), INT2NUM((cresult.ev[i] / cresult.nsamples) * 1000));
      rb_ary_push(list, tmp);
      tmp = 0;
    }
    rb_hash_aset(result, rb_str_new2("eval"), list);
  }

err:
  return result;
}

VALUE cPokerEval;

void
Init_poker_eval_api()
{
    cPokerEval = rb_define_class("PokerEval", rb_cObject);
    rb_define_method(cPokerEval, "eval", t_eval, 1);
}

