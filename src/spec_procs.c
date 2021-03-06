
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"
#include "fight.h"
#include "cmdid.h"

#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
/*
 *  list of room #s
 */
#define Elf_Home     1414
#define Bakery       3009
#define Dump         3030
#define Ivory_Gate   1400

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int top_of_world;
extern struct int_app_type int_app[26];
extern int RacialMax[][MAX_CLASS];

extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
//extern struct spell_info_type spell_info[];
extern const char *dirs[]; 



/* Data declarations */

struct social_type 
{
  char *cmd;
  int next_line;
};

/*
  Brian:  I moved all your stuff to spec_procs2.c

*/



/*************************************/
/* predicates for find_path function */

int is_target_room_p(int room, void *tgt_room)
{
  return room == *( (int *)tgt_room );
}

int named_object_on_ground(int room, void *c_data)
{
  char *name = (char *)c_data;
  return 0 != get_obj_in_list(name, real_roomp(room)->contents);
}

/* predicates for find_path function */
/*************************************/



/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

char *how_good(int percent)
{
  static char buf[256];
  
  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else
    strcpy(buf, " (Superb)");
  
  return (buf);
}

int GainLevel(struct char_data *ch, int iClass)
{

  if (GET_EXP(ch)>=
     titles[iClass][GET_LEVEL(ch, iClass)+1].exp) {
     if (GET_LEVEL(ch, iClass) < RacialMax[GET_RACE(ch)][iClass]) {

       send_to_char("You raise a level!\n\r", ch);
       advance_level(ch, iClass);
       set_title(ch);
       return(TRUE);
     }  else {
       send_to_char("You are unable to advance further in this class\n\r", ch);     
     }
  } else {
     send_to_char("You haven't got enough experience!\n\r",ch);
  }
  return(FALSE);
}

struct char_data *FindMobInRoomWithFunction( int room, 
                                             int (*func)( struct char_data *,
                                                          int, const char *,
                                                          struct char_data *,
                                                          int ) )
{
  struct char_data *temp_char, *targ;

  targ = 0;

  if (room > NOWHERE)
  {
    for( temp_char = real_roomp(room)->people; (!targ) && (temp_char);
         temp_char = temp_char->next_in_room)
    {
      
      if (IS_MOB(temp_char))
      {
        if (mob_index[temp_char->nr].func == CASTVF func)
          targ = temp_char;
      }
    }
  }
  else
  {
    return(0);
  }

  return(targ);

}

int MageGuildMaster( struct char_data *ch, int cmd, const char *arg, 
                     struct char_data *mob, int type) 
{
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if( type != EVENT_COMMAND )
    return FALSE;

  if (!ch->skills)
    return(FALSE);

  if (IS_IMMORTAL(ch))
    return(FALSE);

  if (check_soundproof(ch)) 
    return(FALSE);  

  if(type == EVENT_COMMAND)
     guildmaster = mob;
  else
     guildmaster = ch;

  guildmaster = FindMobInRoomWithFunction(ch->in_room, MageGuildMaster); 

  if (!guildmaster) 
    return(FALSE);

  if( cmd != CMD_PRACTISE && cmd != CMD_PRACTICE && cmd != CMD_GAIN ) 
    return(FALSE);

  if (IS_NPC(ch)) 
  {
    act("$N ti dice 'Ti sembro un addestratore di animali ?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
    return(FALSE);
  }

  for (;*arg == ' '; arg++);

  if (HasClass(ch, CLASS_MAGIC_USER) || HasClass(ch,CLASS_SORCERER))  
  {
    if( cmd == CMD_GAIN )
    { 
      if (HasClass(ch,CLASS_MAGIC_USER)) 
      {
        if (GET_LEVEL(ch,MAGE_LEVEL_IND) < GetMaxLevel(guildmaster)-10) 
        { 
          if( GET_EXP( ch ) <
              titles[MAGE_LEVEL_IND][GET_LEVEL(ch, MAGE_LEVEL_IND)+1].exp) 
          {
            act( "Non sei ancora pront$b\n\r", FALSE, ch, 0, guildmaster, 
                 TO_CHAR );
            return FALSE;
          }
          else 
          {
            GainLevel( ch, MAGE_LEVEL_IND );
            return TRUE;
          }
        }
        else 
        {
          send_to_char( "Non posso allenarti, devi cercare qualcun'altro.\n\r",
                        ch );
        }
      }
      else if (HasClass(ch,CLASS_SORCERER)) 
      {
        if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < GetMaxLevel(guildmaster)-10) 
        { 
          if( GET_EXP( ch ) < titles[SORCERER_LEVEL_IND]
                                    [GET_LEVEL(ch, SORCERER_LEVEL_IND)+1].exp) 
          {
            act( "Non sei ancora pront$b\n\r", FALSE, ch, 0, guildmaster, 
                 TO_CHAR );
            return FALSE;
          }
          else 
          {
            GainLevel(ch,SORCERER_LEVEL_IND);
            return TRUE;
          }
        }
        else 
        {
          send_to_char( "Non posso allenarti, devi cercare qualcun'altro.\n\r",
                        ch );
        }
      }
      return(TRUE);       
    }
    else if( cmd == CMD_PRACTICE || cmd == CMD_PRACTISE )
    {
      if( !*arg )
      {
        sprintf( buf,"Hai a disposizione %d sessioni di pratica.\n\r", 
                 ch->specials.spells_to_learn);
        send_to_char(buf, ch);
        send_to_char("Puoi praticare questi spells:\n\r", ch);
        for(i=0; *spells[i] != '\n'; i++) 
        {
          if (HasClass(ch,CLASS_MAGIC_USER)) 
          {
            if( spell_info[ i + 1 ].spell_pointer &&
                spell_info[ i + 1 ].min_level_magic <=
                GET_LEVEL( ch, MAGE_LEVEL_IND ) &&
                spell_info[ i + 1 ].min_level_magic <=
                GetMaxLevel( guildmaster ) - 10 ) 
            {
              sprintf( buf,"[%d] %s %s \n\r",
                       spell_info[i+1].min_level_magic,
                       spells[i],how_good(ch->skills[i+1].learned));
              send_to_char(buf, ch);
            }
          }
          else if (HasClass(ch,CLASS_SORCERER))
          {
            if( spell_info[ i + 1 ].spell_pointer &&
                spell_info[ i + 1 ].min_level_magic <=
                GET_LEVEL( ch, SORCERER_LEVEL_IND ) &&
                spell_info[ i + 1 ].min_level_magic <=
                GetMaxLevel( guildmaster ) - 10 ) 
            {
              sprintf( buf,"[%d] %s %s \n\r",
                       spell_info[i+1].min_level_magic,
                       spells[i],how_good(ch->skills[i+1].learned));
              send_to_char(buf, ch);
            }
          }
        }
        return(TRUE);
      }
      else
      {
        for (;isspace(*arg);arg++);
          number = old_search_block(arg,0,strlen(arg),spells,FALSE);
        if(number == -1) 
        {
          send_to_char( "Non so` nulla di questa magia.\n\r", ch );
          return(TRUE);
        }

        if( ( HasClass( ch, CLASS_MAGIC_USER ) && 
              GET_LEVEL( ch, MAGE_LEVEL_IND ) < 
              spell_info[ number ].min_level_magic ) ||
            ( HasClass( ch, CLASS_SORCERER ) &&
              GET_LEVEL( ch, SORCERER_LEVEL_IND ) <
              spell_info[ number ].min_level_magic ) )
        {
          do_say( guildmaster, 
                  "Non sei abbastanza potente per imparare questa magia.\n\r", 
                  0 );
          return TRUE;
        }

        if( GetMaxLevel( guildmaster ) - 10 < 
            spell_info[ number ].min_level_magic )
        {
          do_say( guildmaster, 
                  "Non sono abbastanza potente per insegnarti questa magia", 
                  0 );
          return TRUE;
        }
        if( ch->specials.spells_to_learn <= 0 ) 
        {
          do_say( guildmaster, "Non puoi imparare altro per ora.", 0 );
          return TRUE;
        }
    
        if( ch->skills[ number ].learned >= 60 )
        {
          do_say( guildmaster, 
                  "Io non posso piu` insegnarti nulla su questa magia. "
                  "Puoi migliorare solo usandola.", 0 );
          return(TRUE);
        }
    
        send_to_char( "Ti alleni per un po`...\n\r", ch);
        ch->specials.spells_to_learn--;

        if( !IS_SET( ch->skills[ number ].flags, SKILL_KNOWN ) ) 
        {
          SET_BIT( ch->skills[ number ].flags, SKILL_KNOWN );
          if( HasClass( ch, CLASS_SORCERER ) )
            SET_BIT( ch->skills[ number ].flags, SKILL_KNOWN_SORCERER );
          else
            SET_BIT( ch->skills[ number ].flags, SKILL_KNOWN_MAGE );
        }
    
        percent = ch->skills[ number ].learned + 
                  int_app[ (int)GET_RINT( ch ) ].learn;
        ch->skills[ number ].learned = MIN( 95, percent );
     
        if( ch->skills[ number ].learned >= 95 ) 
        {
          act( "Tu sei espert$b in questa materia.\n\r", FALSE, ch, 0, 0, 
               TO_CHAR );
        }
        return TRUE;
      }
    }
  }
  return FALSE;
}

int ClericGuildMaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type) 
{

  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if( type != EVENT_COMMAND )
    return FALSE;

  if( !ch->skills )
    return FALSE;

  if( check_soundproof( ch ) )
    return FALSE;

  if( IS_IMMORTAL( ch ) )
    return FALSE;

  guildmaster = FindMobInRoomWithFunction( ch->in_room, ClericGuildMaster );

  if( !guildmaster )
    return FALSE;

  if( cmd != CMD_PRACTISE && cmd != CMD_PRACTICE && cmd != CMD_GAIN && 
      cmd != CMD_GIVE ) 
    return(FALSE);

  if (IS_NPC(ch)) 
  {
    act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
    return(FALSE);
  }

  for (;*arg == ' '; arg++);

  if (HasClass(ch, CLASS_CLERIC)) 
  {
#ifdef QUEST_GAIN
    if( cmd == CMD_GAIN || cmd == CMD_GIVE ) /*gain or give */ 
#else
    if( cmd == CMD_GAIN ) /*gain */
#endif     
    {  
      if (GET_LEVEL(ch,CLERIC_LEVEL_IND) < GetMaxLevel(guildmaster)-10) 
      { 
#ifdef QUEST_GAIN
        MakeQuest(ch, guildmaster, CLERIC_LEVEL_IND, arg, cmd);
#else
        if( GET_EXP(ch) <
            titles[CLERIC_LEVEL_IND][GET_LEVEL(ch, CLERIC_LEVEL_IND)+1].exp ) 
        {
          act( "Non sei ancora pront$b.", FALSE, ch, 0, 0, TO_CHAR );
          return(FALSE);
        }
        else
        {
          GainLevel(ch,CLERIC_LEVEL_IND);
          return(TRUE);
        }
#endif
      }
      else 
      {
        send_to_char("I cannot train you.. You must find another.\n\r",ch);
      }
      return(TRUE); 
    }

    if( !*arg )
    {
      sprintf(buf,"You have got %d practice sessions left.\n\r", 
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].spell_pointer &&
           (spell_info[i+1].min_level_cleric <= 
            GET_LEVEL(ch,CLERIC_LEVEL_IND)) &&
            (spell_info[i+1].min_level_cleric <=
             GetMaxLevel(guildmaster)-10)) 
        {
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_cleric,spells[i],
                  how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);  
        }
      return(TRUE);
    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) 
    {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch,CLERIC_LEVEL_IND) < spell_info[number].min_level_cleric) 
    {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_cleric) 
    {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) 
    {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (ch->skills[number].learned >= 45) 
    {
      send_to_char("You must use this spell to get any better.  I cannot train you further.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;
    
    if (!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) 
    {
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN_CLERIC);     
    }
    
    percent = ch->skills[number].learned+int_app[ (int)GET_RINT(ch)].learn;
    ch->skills[number].learned = MIN(95, percent);
    
    if (ch->skills[number].learned >= 95) 
    {
      send_to_char("You are now learned in this area.\n\r", ch);
    }
    return(TRUE);
  } 
  return(FALSE);
}

int ThiefGuildMaster( struct char_data *ch, int cmd, const char *arg, 
                      struct char_data *mob, int type )
{
  char buf[256];
  struct char_data *guildmaster;
  const char *n_skills[] = 
  {
    "sneak",     /* 1 */
    "hide",
    "steal",
    "backstab",
    "pick",         /* 5 */
    "spy",     
    "retreat", 
    "find trap",
    "disarm trap",
    "\n",
  };
  int percent=0, number=0;
  int sk_num;

  if( !AWAKE( ch ) )
    return(FALSE);  

  if( type != EVENT_COMMAND )
  {
    if( ch->specials.fighting )
    {
      return( fighter( ch, cmd, arg, ch, 0 ) );
    }
    return(FALSE);
  }

  if( !ch->skills )
    return FALSE;

  if( check_soundproof(ch)) 
     return FALSE;

  guildmaster = FindMobInRoomWithFunction(ch->in_room, ThiefGuildMaster);  
  
  for(; *arg==' '; arg++); /* ditch spaces */
  if( cmd == CMD_PRACTISE || cmd == CMD_PRACTICE || cmd == CMD_GAIN ) 
  {
    if( !HasClass( ch, CLASS_THIEF ) ) 
    {
      send_to_char("The ThiefGuildmaster says 'Get out of here!'\n\r",ch);
      return TRUE;
    }
    if( IS_NPC( ch ) ) 
    {
      act( "$N tells you 'What do i look like, an animal trainer?'", FALSE,
           ch, 0, guildmaster, TO_CHAR );
      return(FALSE);
    }
    
    if( cmd == CMD_GAIN )
    {
      if (GET_LEVEL(ch,THIEF_LEVEL_IND) >= GetMaxLevel(guildmaster)-10) 
      {
        send_to_char("You must learn from another, I can no longer train you.\n\r",ch);
        return(TRUE);
      }
      
      if( GET_EXP(ch) <
          titles[THIEF_LEVEL_IND][GET_LEVEL(ch, THIEF_LEVEL_IND)+1].exp )
      {
        send_to_char("You are not yet ready to gain.\n\r", ch);
        return(FALSE);
      }
      else
      {
        GainLevel(ch,THIEF_LEVEL_IND);
        return(TRUE);
      }
    } /* end gain */
  
    if (!arg || (strlen(arg) == 0)) 
    {
      sprintf(buf,"You have got %d practice sessions left.\n\r", 
              ch->specials.spells_to_learn);
      send_to_char(buf,ch);              

      sprintf(buf," sneak           :  %s\n\r",how_good(ch->skills[SKILL_SNEAK].learned));
      send_to_char(buf,ch);
      sprintf(buf," hide            :  %s\n\r",how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf,ch);
      sprintf(buf," steal           :  %s\n\r",how_good(ch->skills[SKILL_STEAL].learned));
      send_to_char(buf,ch);
      sprintf(buf," backstab        :  %s\n\r",how_good(ch->skills[SKILL_BACKSTAB].learned));
      send_to_char(buf,ch);
      sprintf(buf," pick            :  %s\n\r",how_good(ch->skills[SKILL_PICK_LOCK].learned));
      send_to_char(buf,ch);
      sprintf(buf," spy             :  %s\n\r",how_good(ch->skills[SKILL_SPY].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat         :  %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      sprintf(buf," find trap       :  %s\n\r",how_good(ch->skills[SKILL_FIND_TRAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," disarm trap     :  %s\n\r",how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
      send_to_char(buf,ch);

      return(TRUE);
    }
    else 
    {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The Thief Guildmaster says ",ch);
          
      if (number == -1) 
      {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }

      switch(number) 
      {
      case 0:
      case 1:
        sk_num = SKILL_SNEAK;
        break;
      case 2:
        sk_num = SKILL_HIDE;
        break;
      case 3:
        sk_num = SKILL_STEAL;
        break;
      case 4:
        sk_num = SKILL_BACKSTAB;
        break;
      case 5:
        sk_num = SKILL_PICK_LOCK;
        break;
      case 6:
        sk_num = SKILL_SPY;
        break;
      case 7:
        sk_num = SKILL_RETREAT;
        break;
      case 8:
        sk_num = SKILL_FIND_TRAP;
        break;
      case 9:
        sk_num = SKILL_REMOVE_TRAP;
        break;

      default:
        mudlog( LOG_SYSERR, "Strangeness in Thief Guildmaster (%d)", number);
        send_to_char("'Ack!  I feel sick!'\n\r", ch);
        return(TRUE);
      } /* end switch */
    
      if (ch->skills[sk_num].learned > 45) 
      {
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        return(TRUE);
      }

      if (ch->specials.spells_to_learn <= 0) 
      {
        send_to_char 
          ("'You must first use the knowledge you already have.'\n\r",ch);
        return(FALSE);
      }
    
      send_to_char("'This is how you do it...'\n\r",ch);
      ch->specials.spells_to_learn--;

      if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN)) 
      {
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_THIEF);     
      }
    
      percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
      ch->skills[sk_num].learned = MIN(95, percent);
    
      if (ch->skills[sk_num].learned >= 95) 
      {
        send_to_char("'You are now a master of this art.'\n\r", ch);
      }
      return(TRUE);        
    }
  }
  return(FALSE);
}


#if 0

int WarriorGuildMaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type) 
{

  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;

  static char *w_skills[] = {
    "kick",  /* No. 50 */
    "bash",
    "rescue",
    "\n"
    };

  if (!ch->skills) return(FALSE);


  if (IS_IMMORTAL(ch))
    return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, WarriorGuildMaster);

  if (!guildmaster) return(FALSE);

  if ((cmd != 164) && (cmd != 170) && (cmd != 243) && (cmd != 72))
      return(FALSE);

  if (IS_NPC(ch)) {
    act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
    return(FALSE);
  }

  for (;*arg == ' '; arg++);

  if (HasClass(ch, CLASS_WARRIOR)) {

#ifdef QUEST_GAIN
     if (cmd == 243 || cmd == 72)   /* gain or give */
#else
        if (cmd == 243)
#endif
      {     
       if (GET_LEVEL(ch,WARRIOR_LEVEL_IND) < GetMaxLevel(guildmaster)-10) 
       { 
#ifdef QUEST_GAIN
         MakeQuest(ch, guildmaster, WARRIOR_LEVEL_IND, arg, cmd);
#else
     if (GET_EXP(ch)<
         titles[WARRIOR_LEVEL_IND][GET_LEVEL(ch, WARRIOR_LEVEL_IND)+1].exp) 
         {
          send_to_char("You are not yet ready to gain.\n\r", ch);
          return(FALSE);
         } else
         {
          GainLevel(ch,WARRIOR_LEVEL_IND);
          return(TRUE);
         }
#endif

        } else {
          send_to_char("I cannot train you.. You must find another.\n\r",ch);
        }
        return(TRUE); 
     }

    if (!*arg) {
      sprintf(buf,"You have got %d practice sessions left.\n\r", 
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for(i=0; *w_skills[i] != '\n';i++) {
        send_to_char(w_skills[i], ch);
        send_to_char(how_good(ch->skills[i+SKILL_KICK].learned), ch);
        send_to_char("\n\r", ch);
      }
      return(TRUE);
    }
    for (;isspace(*arg);arg++);
    number = search_block(arg, w_skills, FALSE);
    if(number == -1) {
      send_to_char("You do not have ability to practise this skill!\n\r", ch);
      return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (ch->skills[number+SKILL_KICK].learned >= 45) {
      send_to_char("I have taught you all that I can.  Now you must use your skills.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;
    
    SET_BIT(ch->skills[number+SKILL_KICK].flags, SKILL_KNOWN);
    percent = ch->skills[number+SKILL_KICK].learned +
      int_app[GET_RINT(ch)].learn;
    ch->skills[number+SKILL_KICK].learned = MIN(90, percent);
    
    if (ch->skills[number+SKILL_KICK].learned >= 90) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
  } else {
    send_to_char("Oh.. i bet you think you're a fighter??\n\r", ch);
    return(FALSE);
  }
}

#endif

int WarriorGuildMaster(struct char_data *ch, int cmd, const char *arg, 
                       struct char_data *mob, int type)
{
  char buf[256];
  struct char_data *guildmaster;
  const char *n_skills[] = 
  {
    "kick",     /* 1 */
    "bash",
    "rescue",
    "skin",
    "bellow",
    "\n",
  };
  int percent=0, number=0;
  int sk_num;

  if (!AWAKE(ch))
    return(FALSE);  

  if (!cmd) 
  {
    if (ch->specials.fighting) 
    {
      return(fighter(ch, cmd, arg, ch, 0));
    }
    return(FALSE);
  }

  if (!ch->skills) 
    return(FALSE);

  if (check_soundproof(ch)) 
    return(FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, WarriorGuildMaster);  
  
  for(; *arg==' '; arg++); /* ditch spaces */
  if ((cmd==164)||(cmd==170)||cmd==243) 
  {
    if (!HasClass(ch, CLASS_WARRIOR)) 
    {
      send_to_char("The Warrior Guildmaster says 'Get out of here!'\n\r",ch);
      return(TRUE);
    }
    if (IS_NPC(ch)) 
    {
      act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
          ch, 0, guildmaster, TO_CHAR);
      return(FALSE);
    }
    
    if (cmd==243)
    {
      if (GET_LEVEL(ch,WARRIOR_LEVEL_IND) >= GetMaxLevel(guildmaster)-10) 
      {
        send_to_char("You must learn from another, I can no longer train you.\n\r",ch);
        return(TRUE);
      }
      
      if (GET_EXP(ch)<
          titles[WARRIOR_LEVEL_IND][GET_LEVEL(ch, WARRIOR_LEVEL_IND)+1].exp)
      {
        send_to_char("You are not yet ready to gain.\n\r", ch);
        return(FALSE);
      }
      else
      {
        GainLevel(ch,WARRIOR_LEVEL_IND);
        return(TRUE);
      }
    } /* end gain */

  
    if (!arg || (strlen(arg) == 0)) 
    {
      sprintf(buf,"You have got %d practice sessions left.\n\r", 
              ch->specials.spells_to_learn);
      send_to_char(buf,ch);              

      sprintf(buf," kick          :  %s\n\r",how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf,ch);
      sprintf(buf," bash          :  %s\n\r",how_good(ch->skills[SKILL_BASH].learned));
      send_to_char(buf,ch);
      sprintf(buf," rescue        :  %s\n\r",how_good(ch->skills[SKILL_RESCUE].learned));
      send_to_char(buf,ch);
      sprintf(buf," skin          :  %s\n\r",how_good(ch->skills[SKILL_TAN].learned));
      send_to_char(buf,ch);
      sprintf(buf," bellow        :  %s\n\r",how_good(ch->skills[SKILL_BELLOW].learned));
      send_to_char(buf,ch);
      return(TRUE);
    }
    else 
    {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The Warrior Guildmaster says ",ch);
          
      if (number == -1)
      {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }

      switch(number)
      {
       case 0:
       case 1:
        sk_num = SKILL_KICK;
        break;
       case 2:
        sk_num = SKILL_BASH;
        break;
       case 3:
        sk_num = SKILL_RESCUE;
        break;
       case 4:
        sk_num = SKILL_TAN;
        break;
       case 5:
        sk_num = SKILL_BELLOW;
        break;
        
       default:
        mudlog( LOG_SYSERR, "Strangeness in Warrior Guildmaster (%d)", number);
        send_to_char("'Ack!  I feel sick!'\n\r", ch);
        return(TRUE);
      } /* end switch */
    
      if (ch->skills[sk_num].learned > 45) 
      {
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        return(TRUE);
      }

      if (ch->specials.spells_to_learn <= 0) 
      {
        send_to_char( "'You must first use the knowledge you already have.'\n\r",ch);
        return(FALSE);
      }
    
      send_to_char("'This is how you do it...'\n\r",ch);
      ch->specials.spells_to_learn--;

      if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN))
      {
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_WARRIOR);
      }

      percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
      ch->skills[sk_num].learned = MIN(95, percent);
    
      if (ch->skills[sk_num].learned >= 95) 
      {
        send_to_char("'You are now a master of this art.'\n\r", ch);
      }
      return(TRUE);        
    }
  }
  else
  {
    return(FALSE);
  }
}


int dump( struct char_data *ch, int cmd, const char *arg, struct room_data *rp, 
          int type) 
{
  struct obj_data *k;
  char buf[100];
  struct char_data *tmp_char;
  int value=0;
  
  void do_drop(struct char_data *ch, const char *argument, int cmd);
  char *fname(const char *namelist);
  
  for( k = real_roomp(ch->in_room)->contents; k ; 
       k = real_roomp(ch->in_room)->contents)    
  {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,fname(k->name));
    for( tmp_char = real_roomp(ch->in_room)->people; tmp_char;
         tmp_char = tmp_char->next_in_room )
      if( CAN_SEE_OBJ(tmp_char, k) )
        send_to_char(buf,tmp_char);
    extract_obj(k);
  }
  
  if(cmd!=60)
    return(FALSE);
  
  do_drop(ch, arg, cmd);
  
  value = 0;
  
  for( k = real_roomp(ch->in_room)->contents; k ; 
       k = real_roomp(ch->in_room)->contents )
  {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
    for( tmp_char = real_roomp(ch->in_room)->people; tmp_char;
         tmp_char = tmp_char->next_in_room )
    {
      if (CAN_SEE_OBJ(tmp_char, k))
        send_to_char(buf,tmp_char);
    }
    value+=(MIN(1000,MAX(k->obj_flags.cost/4,1)));
    extract_obj(k);
  }
  
  if (value)         
  {
    act( "You are awarded for outstanding performance.", FALSE, ch, 0, 0,
         TO_CHAR);
    act( "$n has been awarded for being a good citizen.", TRUE, ch, 0,0,
         TO_ROOM);
    
    if( GetMaxLevel(ch) < 3 )
      gain_exp( ch, MIN( 100, value ) );
    else
      GET_GOLD(ch) += value;
  }
  return TRUE;
}

int mayor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  
  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
  
  static char *path;
  static int index;
  static bool move = FALSE;
  
  void do_move(struct char_data *ch, const char *argument, int cmd);
  void do_open(struct char_data *ch, const char *argument, int cmd);
  void do_lock(struct char_data *ch, const char *argument, int cmd);
  void do_unlock(struct char_data *ch, const char *argument, int cmd);
  void do_close(struct char_data *ch, const char *argument, int cmd);

  mob->lStartRoom = 0;

  if(type == EVENT_WINTER) {
    GET_POS(ch) = POSITION_STANDING;
    do_shout(ch, "Aieee!   The rats!  The rats are coming!  Aieeee!",0);
    return(TRUE);
  }

  
  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  
  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING)) {
    if (!ch->specials.fighting) return(FALSE);
    return(fighter(ch, cmd, arg, mob, type));
  }
  
  switch (path[index]) {
  case '0' :
  case '1' :
  case '2' :
  case '3' :
    do_move(ch,"",path[index]-'0'+1);
    break;
    
  case 'W' :
    GET_POS(ch) = POSITION_STANDING;
    act("$n awakens and groans loudly.",FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'S' :
    GET_POS(ch) = POSITION_SLEEPING;
    act("$n lies down and instantly falls asleep.",FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'a' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'Hello Honey!'",FALSE,ch,0,0,TO_ROOM);
    act("$n smirks.",FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'b' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'What a view! I must get something done about that dump!'",
        FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'c' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'd' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'Good day, citizens!'", FALSE, ch, 0,0,TO_ROOM);
    break;
    
  case 'e' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'I hereby declare the bazaar open!'",FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'E' :
    if (check_soundproof(ch)) return(FALSE);
    act("$n says 'I hereby declare Midgaard closed!'",FALSE,ch,0,0,TO_ROOM);
    break;
    
  case 'O' :
    do_unlock(ch, "gate", 0);
    do_open(ch, "gate", 0);
    break;
    
  case 'C' :
    do_close(ch, "gate", 0);
    do_lock(ch, "gate", 0);
    break;
    
  case '.' :
    move = FALSE;
    break;
    
  }
  
  index++;
  return FALSE;
}

#define ACT_OVER_21 1
#define ACT_SNICKER 2

struct pub_beers {
  int        container;
  int   contains;
  int   howman; 
  int   actflag;
};

struct pub_beers sold_here[] = {
  {3903, 3902, 6, 1 },
  {3905, 3904, 6, 1 },
  {3907, 3906, 6, 1 },
  {3909, 3908, 6, 3 },
  {3911, 3910, 6, 3 },
  {3913, 3912, 6, 3 },
  {3914, 0, 0, 1 },
  {3930, 0, 0, 0 },
  {3931, 0, 0, 0 },
  {3932, 0, 0, 0 },
  {3102, 0, 0, 0 },
  {-1}
};
  

struct char_data *find_mobile_here_with_spec_proc(
                                     int (*fcn)(  struct char_data *,
                                                  int, const char *,
                                                  struct char_data *,
                                                  int ), int rnumber )
{
  struct char_data *temp_char;

  for (temp_char = real_roomp(rnumber)->people; temp_char ; 
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char) &&
        mob_index[temp_char->nr].func == CASTVF fcn)
      return temp_char;
  return NULL;
}

int eric_johnson(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)

{
  /* if more than one eric johnson exists in a game, it will
     get confused because of the state variables */
#define        E_HACKING        0
#define        E_SLEEPING        1
#define        E_SHORT_BEER_RUN 2
#define        E_LONG_BEER_RUN        3
#define E_STOCK_FRIDGE        4
#define        E_SKYDIVING        5
#define Erics_Lair        3941
#define DanjerKitchen        3904
#define DanjerLiving        3901
#define DanjerPorch        3900
  static int        fighting=0, state=E_HACKING;
  struct obj_data *temp1;
  struct char_data        *eric, *temp_char;
  char        buf[100];

  eric = 0;

  if (check_soundproof(ch)) return(FALSE);

  eric = find_mobile_here_with_spec_proc(eric_johnson, ch->in_room);
  for (temp_char = real_roomp(ch->in_room)->people; (!eric) && (temp_char) ; 
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func == CASTVF eric_johnson)
        eric = temp_char;

  if (ch==eric) {

    if (cmd!=0)
      return FALSE; /* prevent recursion when eric wants to move */

    if (!fighting && ch->specials.fighting) {
      act("$n says 'What the fuck?'", FALSE, eric, 0, 0, TO_ROOM);
      fighting=1;
    }
    if (fighting && !ch->specials.fighting) {
      act("$n says 'I wonder what their problem was.'", FALSE, eric, 0, 0, TO_ROOM);
      fighting=0;
    }
    if (fighting) {
      struct char_data        *target = eric->specials.fighting;
      act("$n yells for help.", FALSE, eric, 0, 0, TO_ROOM);
      act("$n throws some nasty judo on $N.", FALSE, eric, 0, target, TO_NOTVICT);
      act("$n throws some nasty judo on you.", FALSE, eric, 0, target, TO_VICT);
      if( damage(eric, target, dice(2,4), TYPE_HIT) != VictimDead )
      {
        if (!saves_spell(target, SAVING_SPELL)) 
        {
          struct affected_type af;
          af.type = SPELL_SLEEP;
          af.duration = 2;
          af.modifier = 0;
          af.location = APPLY_NONE;
          af.bitvector = AFF_SLEEP;
          affect_join(target, &af, FALSE, FALSE);
          if (target->specials.fighting)
            stop_fighting(target);
          if (eric->specials.fighting)
            stop_fighting(eric);
          act("$N is out cold.", FALSE, eric, 0, target, TO_NOTVICT);
          act("You are out cold.", FALSE, eric, 0, target, TO_VICT);
          GET_POS(target) = POSITION_SLEEPING;
          RemHated(eric, target);
          RemHated(target, eric);
        }
      }
      return FALSE;
    }

    switch(state) {
    case E_HACKING:
      if (GET_POS(eric)==POSITION_SLEEPING) {
        do_wake(eric, "", -1);
        return TRUE;
      }
      break;
    case E_SLEEPING:
      if (GET_POS(eric)!=POSITION_SLEEPING) {
        act("$n says 'Go away, I'm sleeping'", FALSE, eric, 0,0, TO_ROOM);
        do_sleep(eric, "", -1);
        return TRUE;
      }
      break;
    default:
      if (GET_POS(eric)==POSITION_SLEEPING) {
        do_wake(eric, "", -1);
        return TRUE;
      } else if (GET_POS(eric)!=POSITION_STANDING) {
        do_stand(eric, "", -1);
        return TRUE;
      }
      break;
    }

    switch(state)
    {
      const char *s;
     case E_SLEEPING:
      if (time_info.hours>9 && time_info.hours<12)
      {
        do_wake(eric, "", -1);
        act("$n says 'Ahh, that was a good night's sleep'", FALSE, eric,
            0,0, TO_ROOM);
        state = E_HACKING;
        return TRUE;
      }
      return TRUE;
      break;
     case E_HACKING:
      if (eric->in_room != Erics_Lair)
      {
        /* he's not in his lair, get him there. */
        int dir;
        if (eric->in_room == DanjerLiving)
        {
          do_close(eric, "front",0);
          do_lock(eric, "front",0);
        }
        dir = choose_exit_global(eric->in_room, Erics_Lair, -100);
        if (dir<0)
        {
          if (eric->in_room == DanjerPorch)
          {
            do_unlock(eric, "front",0);
            do_open(eric, "front",0);
            return TRUE;
          }
          dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
        }
        if (dir<0)
        {
          if (dice(1,2)==1)
            act("$n says 'Shit, I'm totally lost.", FALSE, eric, 0,0,TO_ROOM);
          else
            act("$n says 'Can you show me the way back to the DanjerHaus?'",
                FALSE, eric, 0,0, TO_ROOM);
        }
        else
          go_direction(eric, dir);
        
      }
      else
      {
        if (time_info.hours>22 || time_info.hours<3)
        {
          state = E_SLEEPING;
          do_sleep(eric, 0, -1);
          return TRUE;
        }

        do_sit(eric, "", -1);
        if (3==dice(1,5))
        {
          /* he's in his lair, do lair things */
          switch (dice(1,5))
          {
           case 1:
            s = "$n looks at you, then resumes hacking";
            break;
           case 2:
            s = "$n swears at the terminal and resumes hacking";
            break;
           case 3:
            s = "$n looks around and says 'Where's Big Guy?'";
            break;
           case 4:
            s = "$n says 'Dude, RS/6000s suck.'";
            break;
           case 5:
            temp1 = get_obj_in_list_vis(eric, "beer", eric->carrying);
            if (temp1==NULL ||
                temp1->obj_flags.type_flag != ITEM_DRINKCON ||
                temp1->obj_flags.value[1] <= 0)
            {
              s = "$n says 'Damn, out of beer'";
              do_stand(eric, "", -1);
              state = E_SHORT_BEER_RUN;
            }
            else
            {
              do_drink(eric, "beer", -1 /* irrelevant */);
              s = "$n licks his lips";
            }
            break;
           default:
            s = "$n says 'Questo e` impossibile!'";
            break;
          }
          act(s, FALSE, eric, 0, 0, TO_ROOM);
        }
      }
      break;
     case E_SHORT_BEER_RUN:
      if (eric->in_room != DanjerKitchen)
      {
        int        dir;
        dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
        if (dir<0) {
          if (dice(1,3)!=1)
            act("$n says 'Dammit, where's the beer?",
                FALSE, eric, 0,0,TO_ROOM);
          else
            act("$n says 'Christ, who stole my kitchen?'",
                FALSE, eric, 0,0, TO_ROOM);
        } else {
          go_direction(eric, dir);
        }
      } else {
        /* we're in the kitchen, find beer */
        temp1 = get_obj_in_list_vis(eric, "fridge",
                                    real_roomp(eric->in_room)->contents);
        if (temp1==NULL) {
          act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
              0, 0, TO_ROOM);
        } else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
          do_drop(eric, "bottle", -1 /* irrelevant */);
          do_open(eric, "fridge", -1 /* irrelevant */);
        } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "sixpack",
                                                        eric->carrying))) {
          strcpy(buf, "get sixpack fridge");
          command_interpreter(eric, buf);
          if (NULL == get_obj_in_list_vis(eric, "sixpack",
                                          eric->carrying)) {
            act("$n says 'Aw, man.  Someone's been drinking all the beer.",
                FALSE, eric, 0, 0, TO_ROOM);
            do_close(eric, "fridge", -1 /* irrelevant */);
            state = E_LONG_BEER_RUN;
          }
        } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
                                                        eric->carrying))) {
          strcpy(buf, "get beer sixpack");
          command_interpreter(eric, buf);
          if (NULL == get_obj_in_list_vis(eric, "beer",
                                          eric->carrying)) {
            act("$n says 'Well, that one's finished...'", FALSE, eric,
                0, 0, TO_ROOM);
            do_drop(eric, "sixpack", -1 /* irrelevant */);
          }
        } else {
          strcpy(buf, "put sixpack fridge");
          command_interpreter(eric, buf);
          do_close(eric, "fridge", -1 /* irrelevant */);
          state = E_HACKING;
        }
      }
      break;
    case E_LONG_BEER_RUN:
      {
        static struct char_data *andy = 0;
        int        dir;
        const char        **scan,*shopping_list[] =
          { "guinness", "harp", "sierra", "2.harp", NULL };
        
        for (temp_char = character_list; temp_char; temp_char = temp_char->next)
          if (IS_MOB(temp_char))
            if (mob_index[temp_char->nr].func == CASTVF andy_wilcox)
              andy = temp_char;
        
        if (eric->in_room != andy->in_room) {
          if (eric->in_room == DanjerPorch) {
            do_close(eric, "front",0);
            do_lock(eric, "front",0); /* this takes no time */
          } else if (eric->in_room == DanjerLiving) {
              do_unlock(eric, "front",0);
              do_open(eric, "front",0);
            return TRUE; /* this takes one turn */
            }
          dir = choose_exit_global(eric->in_room, andy->in_room, -100);
          if (dir<0) {
            dir = choose_exit_global(eric->in_room, DanjerLiving, -100);
          }
          if (dir<0) {
            act("$n says 'Aw, man.  Where am I going to get more beer?",
                FALSE, eric, 0,0, TO_ROOM);
            state = E_HACKING;
          } else
            go_direction(eric, dir);
        } else {
          for (scan = shopping_list; *scan; scan++) {
            if (NULL == get_obj_in_list_vis(eric, *scan,
                                            eric->carrying)) {
              const char        *s;
              s = (scan[0][1] == '.') ? scan[0]+2 : scan[0];
              sprintf(buf, "buy %s", s);
              command_interpreter(eric, buf);
              if (NULL == get_obj_in_list_vis(eric, *scan,
                                              eric->carrying)) {
                act("$n says 'ARGH, where's my deadbeat roommate with the rent.'", FALSE, eric, 0,0, TO_ROOM);
                act("$n says 'I need beer money.'", FALSE, eric, 0,0, TO_ROOM);
                state = (scan==shopping_list) ? E_HACKING : E_STOCK_FRIDGE;
                return TRUE;
              }
              break;
            }
          }
          if (*scan==NULL || 1 == dice(1,4)) {
            act("$n says 'Catch you later, dude.'", FALSE, eric, 0,0, TO_ROOM);
            state = E_STOCK_FRIDGE;
          }
        }
      }
      break;
    case E_STOCK_FRIDGE:
      if (eric->in_room != DanjerKitchen) {
        int        dir;
        if (eric->in_room == DanjerLiving) {
          do_close(eric, "front",0);
          do_lock(eric, "front",0);
        }
        dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
        if (dir<0) {
          if (eric->in_room == DanjerPorch) {
            do_unlock(eric, "front",0);
            do_open(eric, "front",0);
            return TRUE;
          }
          dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
        }
        if (dir<0) {
          if (dice(1,3)!=1)
            act("$n says 'Dammit, where's the fridge?",
                FALSE, eric, 0,0,TO_ROOM);
          else
            act("$n says 'Christ, who stole my kitchen?'",
                FALSE, eric, 0,0, TO_ROOM);
        } else {
          go_direction(eric, dir);
        }
      } else {
        /* we're in the kitchen, find beer */
        temp1 = get_obj_in_list_vis(eric, "fridge",
                                    real_roomp(eric->in_room)->contents);
        if (temp1==NULL) {
          act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
              0, 0, TO_ROOM);
        } else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
          do_open(eric, "fridge", -1 /* irrelevant */);
        } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
                                                        eric->carrying))) {
          strcpy(buf, "get beer sixpack");
          command_interpreter(eric, buf);
          if (NULL == get_obj_in_list_vis(eric, "beer",
                                          eric->carrying)) {
            act("$n says 'What the hell, I just bought this?!'", FALSE, eric,
                0, 0, TO_ROOM);
            do_drop(eric, "sixpack", -1 /* irrelevant */);
            if (NULL == get_obj_in_list_vis(eric, "sixpack", eric->carrying))
              state = E_HACKING;
          }
        } else {
          strcpy(buf, "put all.sixpack fridge");
          command_interpreter(eric, buf);
          do_close(eric, "fridge", -1 /* irrelevant */);
          state = E_HACKING;
        }
      }
      break;
    }
  }

  return FALSE;
}

/* *******************************************************************
*  General special procedures for mobiles                            *
******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data *npc, const char *cmd, int next_line,
                 int *cur_line, void **thing)
{
  bool ok;

  void do_move(struct char_data *ch, const char *argument, int cmd);
  void do_open(struct char_data *ch, const char *argument, int cmd);
  void do_lock(struct char_data *ch, const char *argument, int cmd);
  void do_unlock(struct char_data *ch, const char *argument, int cmd);
  void do_close(struct char_data *ch, const char *argument, int cmd);

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

    case 'G' :
      *cur_line = next_line;
      return;

    case 'g' :
      *cur_line += next_line;
      return;

    case 'e' :
      act(cmd+1, FALSE, npc, (struct obj_data *)*thing, *thing, TO_ROOM);
      break;

    case 'E' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
      break;

    case 'B' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

    case 'm' :
      do_move(npc, "", *(cmd+1)-'0'+1);
      break;

    case 'w' :
      if (GET_POS(npc) != POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_STANDING;
      break;

    case 's' :
      if (GET_POS(npc) <= POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_SLEEPING;
      break;

    case 'c' :  /* Find char in room */
      *thing = get_char_room_vis(npc, cmd+1);
      ok = (*thing != 0);
      break;

    case 'o' : /* Find object in room */
      *thing = get_obj_in_list_vis(npc, cmd+1, real_roomp(npc->in_room)->contents);
      ok = (*thing != 0);
      break;

    case 'r' : /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd+1));
      break;

    case 'O' : /* Open something */
      do_open(npc, cmd+1, 0);
      break;

    case 'C' : /* Close something */
      do_close(npc, cmd+1, 0);
      break;

    case 'L' : /* Lock something  */
      do_lock(npc, cmd+1, 0);
      break;

    case 'U' : /* UnLock something  */
      do_unlock(npc, cmd+1, 0);
      break;

    case '?' : /* Test a random number */
      if (atoi(cmd+1) <= number(1,100))
        ok = FALSE;
      break;

    default:
      break;
  }  /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}



void npc_steal(struct char_data *ch,struct char_data *victim)
{
  int gold;
  
  if(IS_NPC(victim)) return;
  if(GetMaxLevel(victim)>MAX_MORT) return;

  if (AWAKE(victim) && (number(0,GetMaxLevel(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
    act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


int snake( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
           int type)
{
  void cast_poison( byte level, struct char_data *ch, const char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if( type != EVENT_TICK || !AWAKE(ch))
    return(FALSE);
  
  if( GET_POS(ch) == POSITION_SITTING )
  {
    if( ch->specials.fighting )
      GET_POS(ch) = POSITION_FIGHTING;
    else
      GET_POS(ch) = POSITION_STANDING;
  }

  if( GET_POS(ch) != POSITION_FIGHTING )
    return FALSE;
  
  if( ch->specials.fighting &&
      ch->specials.fighting->in_room == ch->in_room )
  {
    act("$c0010$n morde $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$c0010$n ti morde!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                 ch->specials.fighting, 0 );
    return TRUE;
  }
  return FALSE;
}

int Pungiglione( struct char_data *ch, int cmd, const char *arg, 
                 struct char_data *mob, int type )
{
  void cast_poison( byte level, struct char_data *ch, const char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if( type != EVENT_TICK || !AWAKE(ch))
    return(FALSE);
  
  if( GET_POS(ch) == POSITION_SITTING )
  {
    if( ch->specials.fighting )
      GET_POS(ch) = POSITION_FIGHTING;
    else
      GET_POS(ch) = POSITION_STANDING;
  }

  if( GET_POS(ch) != POSITION_FIGHTING )
    return FALSE;
  
  if( ch->specials.fighting &&
      ch->specials.fighting->in_room == ch->in_room )
  {
    act("$c0010$n punge $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$c0010$n ti punge!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                 ch->specials.fighting, 0 );
    return TRUE;
  }
  return FALSE;
}


int SputoVelenoso( struct char_data *ch, int cmd, const char *arg, 
                   struct char_data *mob, int type )
{
  void cast_poison( byte level, struct char_data *ch, const char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if( type != EVENT_TICK || !AWAKE(ch))
    return(FALSE);
  
  if( GET_POS(ch) == POSITION_SITTING )
  {
    if( ch->specials.fighting )
      GET_POS(ch) = POSITION_FIGHTING;
    else
      GET_POS(ch) = POSITION_STANDING;
  }

  if( GET_POS(ch) != POSITION_FIGHTING )
    return FALSE;
  
  if( ch->specials.fighting &&
      ch->specials.fighting->in_room == ch->in_room )
  {
    act("$c0010$n sputa a $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$c0010$n ti sputa!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                 ch->specials.fighting, 0 );
    return TRUE;
  }
  return FALSE;
}




int PaladinGuildGuard( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  if (!AWAKE(ch))
    return(FALSE);
  
  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg, mob, type);
    } 
  } else if (cmd >= 1 && cmd <= 6) {
    if ((cmd == 2) || (cmd == 1)  || (cmd == 4)) return(FALSE);  
    if (!HasClass(ch, CLASS_PALADIN)) {
      send_to_char
        ("The guard shakes his head, and blocks your way.\n\r", ch);
      act("The guard shakes his head, and blocks $n's way.", 
          TRUE, ch, 0, 0, TO_ROOM);
      return(TRUE);
    }
  }
  return(FALSE);
}

#if 0

int GameGuard( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg,mob,type);
    } 
  } 
  
  if (cmd == 4) {  /* West is field */
    if ((IS_AFFECTED(ch,AFF_TEAM_GREY)) ||
        (IS_AFFECTED(ch,AFF_TEAM_AMBER))){
      send_to_char
        ("The guard wishes you good luck on the field.\n\r", ch);
      return(FALSE);
    } else {
      send_to_char
        ("The guard shakes his head, and blocks your way.\n\r", ch);
      act("The guard shakes his head, and blocks $n's way.", 
          TRUE, ch, 0, 0, TO_ROOM);
      send_to_char
        ("The guard says 'Your not a player! You can't enter the field!'.\n\r", ch);
      return(TRUE);
    }    
    return(FALSE);
  } else return(FALSE);
}

int GreyParamedic(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int types)
{
  struct char_data *vict, *most_hurt;
  
  if (!cmd) {
    if (ch->specials.fighting) {
      return(cleric(ch, 0, "", mob, types));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {
        
        /* Find a dude to do good things upon ! */
        
        most_hurt = real_roomp(ch->in_room)->people;
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room ) {
          if (((float)GET_HIT(vict)/(float)hit_limit(vict) <
               (float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
              && (CAN_SEE(ch, vict)))
            most_hurt = vict;
        }
        if (!most_hurt) return(FALSE); /* nobody here */
        if (IS_AFFECTED(most_hurt,AFF_TEAM_GREY)) {
          
          if ((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
              0.66) {
            if (number(0,5)==0) {
              act("$n shrugs helplessly.", 1, ch, 0, 0, TO_ROOM);
            }
            return TRUE;        /* not hurt enough */
          }
          
          if (!check_soundproof(ch)) {
            if(number(0,4)==0) {
              if (most_hurt != ch) {
                act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
                act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
              }

              if (check_nomagic(ch, 0, 0))
                return(TRUE);

              act("$n utters the words 'judicandus dies'.",
                  1, ch, 0, 0, TO_ROOM);
              cast_cure_light(GetMaxLevel(ch), ch, "", 
                              SPELL_TYPE_SPELL, most_hurt, 0);
              return(TRUE);
            }
          }
        } else { /* Other Team? */
          if (IS_AFFECTED(most_hurt,AFF_TEAM_AMBER)) {

              if (check_nomagic(ch, 0, 0))
                return(TRUE);

              act("$n utters the words 'die punk'.", 
                  1, ch, 0, 0, TO_ROOM);
              cast_cause_light(GetMaxLevel(ch), ch, "", 
                              SPELL_TYPE_SPELL, most_hurt, 0);
              return(TRUE); 
          } else return(FALSE);
        }
      } else {/* I'm asleep or sitting */
        return(FALSE);
      }
    }
  }
  return(FALSE);
}

int AmberParamedic(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict, *most_hurt;
  
  if (!cmd) {
    if (ch->specials.fighting) {
      return(cleric(ch, 0, "",mob,type));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {
        
        /* Find a dude to do good things upon ! */
        
        most_hurt = real_roomp(ch->in_room)->people;
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room ) {
          if (((float)GET_HIT(vict)/(float)hit_limit(vict) <
               (float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
              && (CAN_SEE(ch, vict)))
            most_hurt = vict;
        }
        if (!most_hurt) return(FALSE); /* nobody here */
        if (IS_AFFECTED(most_hurt,AFF_TEAM_AMBER)) {
          
          if ((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
              0.66) {
            if (number(0,5)==0) {
              act("$n shrugs helplessly.", 1, ch, 0, 0, TO_ROOM);
            }
            return TRUE;        /* not hurt enough */
          }
          
          if (!check_soundproof(ch)) {
            if(number(0,4)==0) {
              if (most_hurt != ch) {
                act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
                act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
              }          

              if (check_nomagic(ch, 0, 0))
                return(TRUE);

              act("$n utters the words 'judicandus dies'.", 
                  1, ch, 0, 0, TO_ROOM);
              cast_cure_light(GetMaxLevel(ch), ch, "", 
                              SPELL_TYPE_SPELL, most_hurt, 0);
              return(TRUE);
            }
          }
        } else { /* Other Team? */
          if (IS_AFFECTED(most_hurt,AFF_TEAM_GREY)) {

              if (check_nomagic(ch, 0, 0))
                return(TRUE);

              act("$n utters the words 'die punk'.", 
                  1, ch, 0, 0, TO_ROOM);
              cast_cause_light(GetMaxLevel(ch), ch, "", 
                              SPELL_TYPE_SPELL, most_hurt, 0);
              return(TRUE); 
          } else return(FALSE);
        }
      } else {/* I'm asleep or sitting */
        return(FALSE);
      }
    }
  }
  return(FALSE);
}
#endif


int blink( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);

  
  if (GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3) {
    act("$n blinks.",TRUE,ch,0,0,TO_ROOM);
    cast_teleport( 12, ch, "", SPELL_TYPE_SPELL, ch, 0);
    return(TRUE);
  } else {
    return(FALSE);
  } 
}



int MidgaardCitizen(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (ch->specials.fighting) {
    fighter(ch, cmd, arg,mob,type);
    
    if (check_soundproof(ch)) return(FALSE);
    
    if (number(0,18) == 0) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }
    
    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, 3, MIDGAARD);
    
    return(TRUE);
    
  } else {
    return(FALSE);
  }
}

int ghoul(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tar;
  
  void cast_paralyze( byte level, struct char_data *ch, const char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  tar = ch->specials.fighting;
  
  if( tar && tar->in_room == ch->in_room )
  {
    if( !affected_by_spell( tar, SPELL_PROTECT_FROM_EVIL ) && 
        !IS_AFFECTED( tar, AFF_SANCTUARY ) )
    {
      if (HitOrMiss(ch, tar, CalcThaco(ch, tar)))
      {
        act("$n tocca $N!", 1, ch, 0, tar, TO_NOTVICT);
        act("$n ti tocca!", 1, ch, 0, tar, TO_VICT);
        if (!IS_AFFECTED(tar, AFF_PARALYSIS))
        {
          cast_paralyze( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

int CarrionCrawler(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tar;
  int i;
  
  void cast_paralyze( byte level, struct char_data *ch, const char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i=0;i<8;i++)
  {
    if ((tar = FindAHatee(ch)) == NULL) 
      tar = FindVictim(ch);
    
    if( tar && tar->in_room == ch->in_room )
    {
      if( HitOrMiss( ch, tar, CalcThaco( ch, tar ) ) )
      {
        act("$n tocca $N!", 1, ch, 0, tar, TO_NOTVICT);
        act("$n ti tocca!", 1, ch, 0, tar, TO_VICT);
        if (!IS_AFFECTED(tar, AFF_PARALYSIS))
        {
          cast_paralyze( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

int WizardGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tch, *evil;
  int max_evil;
  
  if (cmd || !AWAKE(ch))
    return (FALSE);
  
  if (ch->specials.fighting) {
    fighter(ch, cmd, arg,mob,type);
    CallForGuard(ch, ch->specials.fighting, 9, MIDGAARD);
  }        
  max_evil = 1000;
  evil = 0;
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }
  
  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
      !check_peaceful(ch, "")) {
    if (!check_soundproof(ch)) {
       act("$n screams 'DEATH!!!!!!!!'", 
           FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}



int vampire(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  void cast_energy_drain(byte level,struct char_data *ch, const char *arg,int type,
                         struct char_data *tar_ch,struct obj_data *tar_obj);
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (ch->specials.fighting && 
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                      ch->specials.fighting, 0);
    if (ch->specials.fighting && 
        (ch->specials.fighting->in_room == ch->in_room)) {
      cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                        ch->specials.fighting, 0);
    }
    return TRUE;
  }
  return FALSE;
}

int wraith(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  void cast_energy_drain( byte level, struct char_data *ch, const char *arg, int type,          struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return(FALSE);

  
  if( ch->specials.fighting && 
      (ch->specials.fighting->in_room == ch->in_room)) 
  {
    act("$n tocca $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n ti tocca!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                      ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}


int shadow(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  void cast_chill_touch( byte level, struct char_data *ch, const char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj );
  void cast_weakness( byte level, struct char_data *ch, const char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (ch->specials.fighting && 
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_chill_touch( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                     ch->specials.fighting, 0);
    if (ch->specials.fighting)
      cast_weakness( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                    ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}



int geyser(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  void cast_geyser( byte level, struct char_data *ch, const char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return FALSE;
  
  if( number( 0, 3 ) == 0 ) 
  {
    act( "Tu erutti.", 1, ch, 0, 0, TO_CHAR);
    cast_geyser( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
    send_to_zone( "Il vulcano emette un lieve rombo.\n\r", ch);
    return TRUE;
  }
  return FALSE;
}


int green_slime( struct char_data *ch, int cmd, const char *arg, 
                 struct char_data *mob, int type)
{
  struct char_data *cons;
  
  void cast_green_slime( byte level, struct char_data *ch, const char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj );
  
  if (cmd || !AWAKE(ch))
    return FALSE;
  
  for( cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room )
    if((!IS_NPC(cons)) && (GetMaxLevel(cons)<LOW_IMMORTAL))
      cast_green_slime( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0);
  
  return FALSE;
}


int DracoLich( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
               int type)
{
  return FALSE;
}


int Drow(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);
    
  if (!ch->specials.fighting) 
  {
    if( !IS_UNDERGROUND(ch) && !IS_DARK( ch->in_room ) && 
        !affected_by_spell( ch, SPELL_GLOBE_DARKNESS ) )
    {
      act("$n uses $s innate powers of darkness",FALSE,ch,0,0,TO_ROOM);
      cast_globe_darkness(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
      return(TRUE);
    }
    if( ch->specials.hunting )
    {
      act("$n uses $s innate powers of levitation",FALSE,ch,0,0,TO_ROOM);
      cast_flying(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
      return(TRUE);
    }
  } /* end not fighting */  
  else 
  {
  
    if( !affected_by_spell( ch->specials.fighting, SPELL_BLINDNESS ) && 
        number(1,100) > 25 )
    {
      /* simulate casting darkness on a person... */
      act( "$n uses $s innate powers of darkness on $N!", FALSE, ch, 0,
           ch->specials.fighting,TO_NOTVICT);
      act( "$n drops a pitch black globe around you!", FALSE, ch, 0,
           ch->specials.fighting, TO_VICT );
      cast_blindness( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 
                      ch->specials.fighting, 0 );
      return TRUE;
    } /* end darkness */

    if( !affected_by_spell( ch->specials.fighting, SPELL_FAERIE_FIRE ) &&
        number(1,100)>50)        
    {
      /* simulate faerie fire */
      act( "$n tries to outline $N with $s faerie fire!", FALSE, ch, 0,
           ch->specials.fighting, TO_NOTVICT );
      act( "$n tries to outline you with a faerie fire glow!", FALSE, ch, 0,
           ch->specials.fighting,TO_VICT );
      cast_faerie_fire( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 
                        ch->specials.fighting, 0 );
      return TRUE;
    } /* end faerie fire */
  } /* end was fighting */

  return( archer( ch, cmd, arg, mob, type ) );
}

int Leader(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return FALSE;
}


int thief(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *cons;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if(GET_POS(ch)!=POSITION_STANDING)return FALSE;

  for(cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room )
    if((!IS_NPC(cons)) && (GetMaxLevel(cons)<LOW_IMMORTAL) && (number(1,5)==1))
        npc_steal(ch,cons); 

  return(FALSE);
}





/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

int guild_guard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  if (!cmd) {
     if (ch->specials.fighting) {
       return(fighter(ch, cmd, arg,mob,type));
     }
  } else {
   
     switch(ch->in_room) {
     case 3017:
        return(CheckForBlockedMove(
               ch, cmd, arg, 3017, 2, CLASS_MAGIC_USER|CLASS_SORCERER));
        break;
     case 3004:
        return(CheckForBlockedMove(ch, cmd, arg, 3004, 0, CLASS_CLERIC));
        break;
     case 3027:
        return(CheckForBlockedMove(ch, cmd, arg, 3027, 1, CLASS_THIEF));
        break;
     case 3021:
        return(CheckForBlockedMove(ch, cmd, arg, 3021, 1, CLASS_WARRIOR));
        break;
    }
  }
  
  return FALSE;
  
}




int Inquisitor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if( cmd || !AWAKE( ch ) )
    return FALSE;

  if( ch->specials.fighting )
  {
    return( fighter( ch, cmd, arg, mob, type ) );
  }

  if(ch->generic == INQ_SHOUT )
  {
    if( !check_soundproof( ch ) )
      do_shout( ch, "NESSUNO vuole l'Inquisizione Spagnola!", 0 );
    ch->generic = 0;
    return TRUE;
  }
  return FALSE;
}

int puff(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *i, *tmp_ch;
  char buf[80];
  
  if(type == EVENT_DWARVES_STRIKE) {
    do_shout(ch, "Ack! Of all the stupid things! Those damned dwarves are on strike again!", 0);
    return(TRUE);
  }

  if(type == EVENT_END_STRIKE) {
    do_shout(ch, "Gee, about time those dwarves stopped striking!", 0);
    return(TRUE);
  }

  if(type == EVENT_DEATH) {
    do_shout(ch, "Ack! I've been killed! Have some God Load me again!!!", 0);
    return(TRUE);
  }

  if(type == EVENT_SPRING) {
    do_shout(ch, "Ahhh, spring is in the air.", 0);
    return(TRUE);
  }

  if (cmd)
    return FALSE;

  if (ch->generic == 1) 
  {
    do_shout(ch, "When will we get there?", 0);
    ch->generic = 0;
  }


  if (check_soundproof(ch)) 
    return(FALSE);
    
  switch (number(0, 250))
  {
    case 0:
      sprintf(buf,"Anyone know where I am at?\n");
      do_say(ch, buf, 0);
      return(1);
    case 1:
      do_say(ch, "How'd all those fish get up here?", 0);
      return(1);
    case 2:
      do_say(ch, "I'm a very female dragon.", 0);
      return(1);
    case 3:
      do_say(ch, "Haven't I seen you at the Temple?", 0);
      return(1);
    case 4:
      do_shout(ch, "Bring out your dead, bring out your dead!", 0);
      return(1);
    case 5:
      do_emote(ch, "gropes you.", 0);
      return(1);
    case 6:
      do_emote(ch, "gives you a long and passionate kiss.  It seems to last forever.", 0);
      return(1);
    case 7:
      {
        for (i = character_list; i; i = i->next) {
          if (!IS_NPC(i)) {
            if (number(0,5)==0) {
              if (!strcmp(GET_NAME(i),"Celestian")) {
                do_shout(ch,"Celestian, come ravish me now!",0);
              } else if (!strcmp(GET_NAME(i), "Fiona")) {
                do_shout(ch,"I'm Puff the PMS dragon!",0);
              } else if (!strcmp(GET_NAME(i), "Stranger")) {
                do_shout(ch, "People are strange, when they're with Stranger!",0);
              } else if (!strcmp(GET_NAME(i), "God")) {
                do_shout(ch, "God!  Theres only room for one smartass robot on this mud!",0);
              } else if (GET_SEX(i)==SEX_MALE) {
                sprintf(buf,"Hey, %s, how about some MUDSex?",GET_NAME(i));
                do_say(ch,buf,0);
              } else {
                sprintf(buf,"I'm much prettier than %s, don't you think?",GET_NAME(i));
                do_say(ch,buf,0);
              }
            }
          }
          break;
        }
      }
      return(1);
    case 8:
      do_say(ch, "Celestian is my hero!", 0);
      return(1);
    case 9:
      do_say(ch, "So, wanna neck?", 0);
      return(1);
    case 10:
      {
        tmp_ch = (struct char_data *)FindAnyVictim(ch);
        if (!IS_NPC(ch)) {
          sprintf(buf, "Party on, %s", GET_NAME(tmp_ch)); 
          do_say(ch, buf, 0);
          return(1);
        } else {
          return(0);
        }
      }
    case 11:
      if (!number(0,30))
        do_shout(ch, "NOT!!!", 0);
      return(1);
    case 12:
      do_say(ch, "Bad news.  Termites.", 0);
      return(1);
    case 13:
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0,30)==0) {
            sprintf(buf, "%s shout I love Joy!",GET_NAME(i));
            do_force(ch, buf, 0);
            sprintf(buf, "%s bounce", GET_NAME(i));
            do_force(ch, buf, 0);
            do_restore(ch, GET_NAME(i), 0);
            return(TRUE);
          }
        }
      }
      return(1);
    case 14:
      do_say(ch, "I'll be back.", 0);
      return(1);
    case 15:
      do_say(ch, "Aren't wombat's so cute?", 0);
      return(1);
    case 16:
      do_emote(ch, "fondly fondles you.", 0);
      return(1);
    case 17:
      do_emote(ch, "winks at you.", 0);
      return(1);
    case 18:
      do_say(ch, "This mud is too silly!", 0);
      return(1);
    case 19:
      do_say(ch, "If the Mayor is in a room alone, ", 0);
      do_say(ch, "Does he say 'Good morning citizens.'?",0);
      return(0);
    case 20:
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0,30)==0) {
            sprintf(buf, "Top of the morning to you %s!", GET_NAME(i));
            do_shout(ch, buf, 0);
            return(TRUE);
          }
        }
      }
      break;
    case 21:
      for (i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0,3)==0) {
            sprintf(buf, "Pardon me, %s, but are those bugle boy jeans you are wearing?", GET_NAME(i));
            do_say(ch, buf, 0);
            return(TRUE);
          }
        }
      }
      break;
    case 22:
      for (i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0,3)==0) {
            sprintf(buf, "Pardon me, %s, but do you have any Grey Poupon?", GET_NAME(i));
            do_say(ch, buf, 0);
            return(TRUE);
          }
        }
      }
      break;
    case 23:
      if (number(0,80)==0) {
        do_shout(ch, "Where are we going?", 0);
        ch->generic = 1;
      }
      break;
    case 24:
      do_say(ch, "Blackstaff is a wimp!", 0);
      return(TRUE);
      break;
    case 25:
      do_say(ch, "Better be nice or I will user spellfire on you!", 0);
      return(TRUE);
      break;
    case 26:
      if (number(0,100)==0)
        do_shout(ch, "What is the greatest joy?", 0);
      break;
    case 27:
      do_say(ch, "Have you see Seneca ? Gads he is slow...", 0);
      return(TRUE);
    case 28:
      if (number(0,50))
        do_shout(ch, "SAVE!  I'm running out of cute things to say!", 0);
        do_force(ch, "all save", 0);
      return(TRUE);
    case 29:
      do_say(ch, "I hear Strahd is a really mean vampire.", 0);
      return(TRUE);
    case 30:
      do_say(ch, "I heard there was a sword that would kill frost giants.", 0);
      return(TRUE);
    case 31:
      do_say(ch, "Hear about the sword that kills Red Dragons?", 0);
      return(TRUE);
    case 32:
      if (number(0,100)==0) {
        do_shout(ch, "Help yourself and help a newbie!", 0);
        return(TRUE);
      }
      break;
    case 33:
      if (number(0,100)==0) {
        /* do_shout(ch, "Kill all other dragons!", 0); */
        return(TRUE);
      }
      break;
    case 34:
      if (number(0,50)==0) {
        for (i = character_list; i; i=i->next) {
          if (mob_index[i->nr].func == CASTVF Inquisitor) {
               do_shout(ch, "I wasn't expecting the Spanish Inquisition!", 0);
             i->generic = INQ_SHOUT;
             return(TRUE);
           }
        }
        return(TRUE);
      }
      break;
    case 35:
      do_say(ch, "Are you crazy, is that your problem?", 0);
      return(TRUE);
    case 36:
      for (i = real_roomp(ch->in_room)->people; i; i=i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0,3)==0) {
            sprintf(buf, "%s, do you think I'm going bald?",GET_NAME(i));
            do_say(ch, buf, 0);
            return(TRUE);
          }
        }
      }
      break;
    case 37:
      do_say(ch, "This is your brain.", 0);
      do_say(ch, "This is MUD.", 0);
      do_say(ch, "This is your brain on MUD.", 0);
      do_say(ch, "Any questions?", 0);
      return(TRUE);
    case 38:
      for (i = character_list; i; i=i->next) 
      {
        if (!IS_NPC(i)) 
        {
          if (number(0,20) == 0) 
          {
            if (i->in_room != NOWHERE) 
            {
              sprintf(buf, "%s save", GET_NAME(i));
              do_force(ch, buf, 0);
              return(TRUE);
            }
          }
        }
      }
      return(TRUE);
    case 39:
      do_say(ch, "I'm Puff the Magic Dragon, who the hell are you?", 0);
      return(TRUE);
    case 40:
      do_say(ch, "Attention all planets of the Solar Federation!", 0);
      do_say(ch, "We have assumed control.", 0);
      return(TRUE);
    case 41:
      if (number(0,50)==0) {
        do_shout(ch, "We need more explorers!", 0);
        return(TRUE);
      }
      break;
    case 42:
      if (number(0,50)==0) {
        do_shout(ch, "Pray to Joy, he might be in a good mood!", 0);
        return(TRUE);
      }
      break;
    case 43:
      do_say(ch, "Pardon me boys, is this the road to Great Cthulhu?", 0);
      return(TRUE);
    case 44:
      do_say(ch, "May the Force be with you... Always.", 0);
      return(TRUE);
    case 45:
      do_say(ch, "Eddies in the space time continuum.", 0);
      return(TRUE);
    case 46:
      do_say(ch, "Quick!  Reverse the polarity of the neutron flow!", 0);
      return(TRUE);
    case 47:
      if (number(0,50) == 0) {
        do_shout(ch, "Someone pray to Blackmouth, he is lonely.", 
                 0);
        return(TRUE);
      }
      break;
    case 48:
      do_say(ch, "Shh...  I'm beta testing.  I need complete silence!", 0);
      return(TRUE);
    case 49:
      do_say(ch, "Do you have any more of that Plutonium Nyborg!", 0);
      return(TRUE);
    case 50:
      do_say(ch, "I'm the real implementor, you know.", 0);
      return(TRUE);
    case 51:
      do_emote(ch, "moshes into you almost causing you to fall.", 0);
      return(TRUE);
    case 52:
      if (!number(0,30))
        do_shout(ch, "Everybody pray to Ator!", 0);
      return(TRUE);
    case 53:
      do_say(ch, "You know I always liked you the best don't you?", 0);
      do_emote(ch, "winks seductively at you.", 0);
      return(TRUE);
    case 54:
      if (!number(0,30))
        do_shout(ch, "Ack! Who prayed to Wert!", 0);      
      return(TRUE);

    default:
      return(0);
  }
  return FALSE;
}

int regenerator( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if( type == EVENT_TICK )
  {
    if( GET_HIT( ch ) < GET_MAX_HIT( ch ) ) 
    {
      GET_HIT(ch) += 9;
      GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

      act("$n rigenera.", TRUE, ch, 0, 0, TO_ROOM);
      return(TRUE);
    }
  }
  return FALSE;
}

int replicant( struct char_data *ch, int cmd, const char *arg, 
               struct char_data *mob1, int type )
{
  struct char_data *mob;

  if( type == EVENT_TICK )
  {
    if (GET_HIT(ch) < GET_MAX_HIT(ch)) 
    {
      act( "Gocce del sangue di $n cadono a terra e si trasformano!",
           TRUE, ch, 0, 0, TO_ROOM);
      mob = read_mobile(ch->nr, REAL);
      char_to_room(mob, ch->in_room);
      act( "Ora ci sono due opponenti integri di fronte a te.", 
           TRUE, ch, 0, 0, TO_ROOM );
      GET_HIT(ch) = GET_MAX_HIT(ch);
      return TRUE;
    }
  }

  return FALSE;
}

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4
        
int Tytan( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
           int type)
{
  struct char_data *vict;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting)
  {
    return(magic_user(ch, cmd, arg, mob, type));
  }
  else
  {
    switch(ch->generic)
    {
     case TYT_NONE:
      if( ( vict = FindVictim( ch ) ) )
      {
        ch->generic = TYT_CIT;
        SetHunting(ch, vict);
      }
      break;
     case TYT_CIT:
      if (ch->specials.hunting)
      {
        if (IS_SET(ch->specials.act, ACT_AGGRESSIVE))
        {
          REMOVE_BIT(ch->specials.act, ACT_AGGRESSIVE);
        }
        if (ch->in_room == ch->specials.hunting->in_room)
        {
          act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
          ch->generic = TYT_WHAT;
        }
      } 
      else
      {
        ch->generic = TYT_NONE;
      }
      break;
     case TYT_WHAT:
      if (ch->specials.hunting)
      {
        if (ch->in_room == ch->specials.hunting->in_room)
        {
          act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
          ch->generic = TYT_TELL;
        }
      } 
      else
      {
        ch->generic = TYT_NONE;
      }
      break;
     case TYT_TELL:
      if (ch->specials.hunting)
      {
        if (ch->in_room == ch->specials.hunting->in_room)
        {
          act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
          ch->generic = TYT_HIT;
        }
      }
      else
      {
        ch->generic = TYT_NONE;
      }
      break;
     case TYT_HIT:
      if (ch->specials.hunting)
      {
        if (ch->in_room == ch->specials.hunting->in_room)
        {
          if (!check_peaceful(ch, "The Tytan screams in anger"))
          {
            if (!IS_SET(ch->specials.act, ACT_AGGRESSIVE))
            {
              SET_BIT(ch->specials.act, ACT_AGGRESSIVE);
            }
            ch->generic = TYT_NONE;
            hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
          }
          else
          {
            ch->generic = TYT_CIT;
          }
        }
      }
      else
      {
        ch->generic = TYT_NONE;
      }
      break;
     default:
      ch->generic = TYT_NONE;
    }
  }
  return(FALSE);
}

int AbbarachDragon(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  struct char_data *targ;

  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if( !ch->specials.fighting )
  {
    targ = (struct char_data *)FindAnyVictim(ch);
    if (targ && !check_peaceful(ch, "")) 
    {
      hit(ch, targ, TYPE_UNDEFINED);
      act("You have now payed the price of crossing.", TRUE, ch, 0, 0, TO_ROOM);
      return(TRUE);
    }
  }
  else 
  {
     return(BreathWeapon(ch, cmd, arg,mob,type));
  }
  return FALSE;
}


int fido(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  register struct obj_data *i, *temp, *next_obj, *next_r_obj;
  register struct char_data *v, *next;
  register struct room_data *rp;
  char found = FALSE;
  
  if (cmd || !AWAKE(ch))
    return(FALSE);

  if ((rp = real_roomp(ch->in_room)) == 0)
    return(FALSE);
    
  for (v = rp->people; (v && (!found)); v = next) {
    next = v->next_in_room;
    if ((IS_NPC(v)) && (mob_index[v->nr].iVNum == 100) &&
        CAN_SEE(ch, v)) {  /* is a zombie */
      if (v->specials.fighting)
        stop_fighting(v);
      make_corpse( v, 0 );
      extract_char(v);
      found = TRUE;
    } 
  }
  
  
  for (i = real_roomp(ch->in_room)->contents; i; i = next_r_obj) {
    next_r_obj = i->next_content;
    if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for(temp = i->contains; temp; temp=next_obj)        {
        next_obj = temp->next_content;
        obj_from_obj(temp);
        obj_to_room(temp,ch->in_room);
      }
      extract_obj(i);
      return(TRUE);
    }
  }
  return(FALSE);
}



int janitor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct obj_data *i;
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && 
        ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
         (i->obj_flags.cost <= 10))) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      
      obj_from_room(i);
      obj_to_char(i, ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

int tormentor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (!cmd) return(FALSE);

  if (IS_IMMORTAL(ch)) return(FALSE);

  if (!IS_PC(ch)) return(FALSE);

  return(TRUE);

}

int RustMonster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;
  struct obj_data *t_item;
  int t_pos;

  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  /*
   **   find a victim
   */
  vict = ch->specials.fighting;
  
  if( !vict )
    vict = FindVictim(ch);
  
  if (!vict)
    return(FALSE);

  /*
   **   choose an item of armor or a weapon that is metal
   **  since metal isn't defined, we'll just use armor and weapons   
   */

  /*
   **  choose a weapon first, then if no weapon, choose a shield,
   **  if no shield, choose breast plate, then leg plate, sleeves,
   **  helm
   */

  if (vict->equipment[WIELD])
  {
    t_item = vict->equipment[WIELD];
    t_pos = WIELD;
  }
  else if (vict->equipment[WEAR_SHIELD])
  {
    t_item = vict->equipment[WEAR_SHIELD];
    t_pos = WEAR_SHIELD;
  }
  else if (vict->equipment[WEAR_BODY])
  {
    t_item = vict->equipment[WEAR_BODY];
    t_pos = WEAR_BODY;
  }
  else if (vict->equipment[WEAR_LEGS])
  {
    t_item = vict->equipment[WEAR_LEGS];
    t_pos = WEAR_LEGS;
  }
  else if (vict->equipment[WEAR_ARMS])
  {
    t_item = vict->equipment[WEAR_ARMS];
    t_pos = WEAR_ARMS;
  }
  else if (vict->equipment[WEAR_HEAD])
  {
    t_item = vict->equipment[WEAR_HEAD];
    t_pos = WEAR_HEAD;
  }
  else
  {
    return(FALSE);
  }

  /*
   **  item makes save (or not)
   */
  if (DamageOneItem(vict, ACID_DAMAGE, t_item))
  {
    t_item = unequip_char(vict, t_pos);
    if (t_item)
    {
      /*
       **  if it doesn't make save, falls into a pile of scraps
       */
      MakeScrap(vict,NULL, t_item);
    }
  }

  return(FALSE);

}

int temple_labrynth_liar(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  void do_say(struct char_data *ch, const char *argument, int cmd);
  
  if (cmd || !AWAKE(ch))
    return(0);

  if (check_soundproof(ch)) return(FALSE);
  
  switch (number(0, 15)) {
    case 0:
      do_say(ch, "I'd go west if I were you.", 0);
      return(1);
    case 1:
      do_say(ch, "I heard that Vile is a cute babe.", 0);
      return(1);
    case 2:
      do_say(ch, "Going east will avoid the beast!", 0);
      return(1);
    case 4:
      do_say(ch, "North is the way to go.", 0);
      return(1);
    case 6:
      do_say(ch, "Dont dilly dally go south.", 0);
      return(1);
    case 8:
      do_say(ch, "Great treasure lies ahead", 0);
      return(1);
    case 10:
      do_say(ch, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
      return(1);
    case 12:
      do_say(ch, "I am a very clever liar.", 0);
      return(1);
    case 14:
      do_say(ch, "Loki is a really great guy!", 0);
      do_say(ch, "Well.... maybe not...", 0);
      return(1);
    default:
      do_say(ch, "Then again I could be wrong!", 0);
      return(1);
    }
}

int temple_labrynth_sentry(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tch;
  int counter;
  
  void cast_fireball( byte level, struct char_data *ch, const char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj );
  void do_say(struct char_data *ch, const char *argument, int cmd);
  
  if(cmd || !AWAKE(ch)) return FALSE;
  
  if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
  
  if(!ch->specials.fighting) return FALSE;

  if (check_soundproof(ch)) return(FALSE);
    
  /* Find a dude to do very evil things upon ! */
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if( GetMaxLevel(tch)>10 && CAN_SEE(ch, tch)) {
      act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
        do_say(ch, "You will die for your insolence, pig-dog!", 0);
        for ( counter = 0 ; counter < 4 ; counter++ )
          if (check_nomagic(ch, 0, 0))
            return(FALSE);
          if ( GET_POS(tch) > POSITION_SITTING)
            cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
          else
            return TRUE;
        return TRUE;
      }
    else
      {
        act("The sentry looks concerned and continues to push you away",
            1, ch, 0, 0, TO_ROOM);
        do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
      }
  }
  return TRUE;
}

#if 0
#define WW_LOOSE 0
#define WW_FOLLOW 1

int Whirlwind (struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tmp;
  const char *names[] = { "Loki", "Belgarath", 0};
  int i = 0;
  
  if (ch->in_room == -1) return(FALSE);
  
  if (cmd == 0 && ch->generic == WW_LOOSE)  {   
    for (tmp = real_roomp(ch->in_room)->people; tmp ; tmp = tmp->next_in_room) {
      while (names[i])  {
        if ( !strcmp(GET_NAME(tmp), names[i] ) && ch->generic == WW_LOOSE)  {
          /* start following */
          if (circle_follow(ch, tmp)) 
            return(FALSE);
          if (ch->master)
            stop_follower(ch);
          add_follower(ch, tmp);
          ch->generic = WW_FOLLOW;
        }
        i++;
      }
    }
    if (ch->generic == WW_LOOSE && !cmd )  {
      act("The $n suddenly dissispates into nothingness.",0,ch,0,0,TO_ROOM);   
      extract_char(ch);      
    }
  }
}
#endif

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

int NudgeNudge(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  struct char_data *vict;
  
  if (cmd || !AWAKE(ch))
    return (FALSE);
  
  if (ch->specials.fighting) {
    return(FALSE);
  }

  
  switch(ch->generic) {
  case NN_LOOSE:
     /*
     ** find a victim
     */
    vict = FindVictim(ch);
    if (!vict)
      return(FALSE);
    /* start following */
    if (circle_follow(ch, vict)) {
      return(FALSE);
    }
    if (ch->master)
      stop_follower(ch);
    add_follower(ch, vict);
    ch->generic = NN_FOLLOW;
    if (!check_soundproof(ch))
       do_say (ch, "Good Evenin' Squire!" , 0 );
    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
    break;
  case NN_FOLLOW:
    switch(number(0,20)) {
    case 0:
      if (!check_soundproof(ch))
        do_say  (ch, "Is your wife a goer?  Know what I mean, eh?", 0 );
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 1:
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      if (!check_soundproof(ch))
        do_say  (ch, "Say no more!  Say no MORE!", 0);   
      break;
    case 2:
      if (!check_soundproof(ch)) {
        do_say  (ch, "You been around, eh?", 0);
        do_say  (ch, "...I mean you've ..... done it, eh?", 0);
      }
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 3:
      if (!check_soundproof(ch))
        do_say  (ch, "A nod's as good as a wink to a blind bat, eh?", 0);  
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 4:
      if (!check_soundproof(ch))
        do_say  (ch, "You're WICKED, eh!  WICKED!", 0);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 5:
      if (!check_soundproof(ch))
        do_say  (ch, "Wink. Wink.", 0);
      break;
    case 6:
      if (!check_soundproof(ch))
        do_say  (ch, "Nudge. Nudge.", 0);
      break;
    case 7:
    case 8:
      ch->generic = NN_STOP;
      break;
    default:
      break;
    }
    break;
  case NN_STOP:
    /*
    **  Stop following
    */
    if (!check_soundproof(ch))
      do_say(ch, "Evening, Squire", 0);
    stop_follower(ch);
    ch->generic = NN_LOOSE;
    break;
  default:
    ch->generic = NN_LOOSE;
    break;
  }  
  return(TRUE);
}

int AGGRESSIVE(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *i, *next;

  if (cmd || !AWAKE(ch)) 
    return(FALSE);

  if (check_peaceful(ch, ""))
    return FALSE;

  if (ch->in_room > -1) 
  {
    for (i = real_roomp(ch->in_room)->people; i; i = next) 
    {
      next = i->next_in_room;
      if (GET_RACE(i) != GET_RACE(ch)) 
      {
        if (!IS_IMMORTAL(i) && CAN_SEE(ch, i)) 
        {
          hit(ch, i, TYPE_UNDEFINED);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

int citizen(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (ch->specials.fighting) {
    fighter(ch, cmd, arg,mob,type);

    if (check_soundproof(ch)) return(FALSE);

    if (number(0,18) == 0) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }
  }
  return(FALSE);
}

int MidgaardCityguard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
   return(GenericCityguardHateUndead(ch, cmd, arg, mob, MIDGAARD));
}



#define ONE_RING 1105
int Ringwraith( struct char_data *ch, int cmd, const char *arg, 
                struct char_data *mob, int type )
{
  static char      buf[ 256 ];
  struct char_data *victim;
  static int       howmanyrings = 0;
  struct obj_data  *ring;
  struct wraith_hunt 
  {
    int        ringnumber;
    int        chances;
  } *wh;
  int rnum,dir;

  if( !AWAKE( mob ) || !IS_NPC( mob ) || type == EVENT_COMMAND ) 
  {
    return FALSE;
  }
  
  if( type == EVENT_TICK )
  {
    if( ch->specials.fighting )
    {
      if( GET_POS( ch ) > POSITION_SLEEPING &&
         GET_POS( ch ) < POSITION_FIGHTING ) 
      {
        do_stand( ch, "", 0 );
      } 
      else
      {
        wraith( ch, cmd, arg, mob, type );
      }
      
      return FALSE ;
    }

    /* how many one rings are in the game? */
    if( howmanyrings == 0 )
    { 
      if( ch->generic == 0 )
      {
        howmanyrings = 0;
        get_obj_vis_world( ch, "999.primo anello.", &howmanyrings );
        ch->generic = 20;
      }
      else
        ch->generic--;
    }

    if( howmanyrings > 0 )
    {
      /* does our ringwraith have his state info? */
      if( ch->act_ptr == 0 ) 
      { 
        ch->act_ptr = (struct wraith_hunt *)malloc(sizeof(*wh)); 
        wh = (struct wraith_hunt *)ch->act_ptr;
        wh->ringnumber = 0;
      } 
      else
      {
        wh = (struct wraith_hunt *)ch->act_ptr;
      }
  
      if( !wh->ringnumber )
      { 
        /* is he currently tracking a ring */
        wh->chances=0;
        wh->ringnumber = number( 1, howmanyrings++ );
      }
  
      sprintf( buf, "%d.primo anello.", (int)wh->ringnumber );
      /* where is this ring? */
      if( ( ring = get_obj_vis_world( ch, buf, NULL ) ) != NULL )
      {
        rnum = room_of_object( ring );
  
        if( rnum != ch->in_room )
        {
          dir = find_path( ch->in_room, is_target_room_p, &rnum,
                           -5000, 0 );
          if( dir < 0 ) 
          { 
            /* we can't find the ring */
            wh->ringnumber = 0;
            return FALSE;
          }
          go_direction(ch, dir);
        }
        else if( ( victim = char_holding( ring ) ) )
        {
          /* the ring is in the same room! */
          if( victim == ch ) 
          {
            obj_from_char( ring );
            extract_obj( ring );
            wh->ringnumber = 0;
            howmanyrings--;
            ch->generic = 20;
            act( "$n brilla contento.", FALSE, ch, NULL, victim, TO_ROOM );
          }  
          else 
          {
            switch( wh->chances )
            {
             case 0:
              do_wake( ch, GET_NAME( victim ), 0);
              if( !check_soundproof( ch ) )
                act( "$c0015[$c0005$n$c0015] dice '$N, dammi l'Anello'.",
                     FALSE, ch, NULL, victim, TO_ROOM );
              else 
                act( "$n ti infila un dito nelle costole.", FALSE, ch, NULL, 
                     victim, TO_ROOM);
              wh->chances++;
              break;
             case 1:
              if( IS_NPC( victim ) )
              {
                act( "$N da` velocemente l'Anello a $n.", FALSE, ch, NULL,
                     victim, TO_ROOM);
                if (ring->carried_by)
                  obj_from_char(ring);
                else if (ring->equipped_by)
                  unequip_char(victim, ring->eq_pos);
                obj_to_char(ring, ch);
              } 
              else 
              {
                if (!check_soundproof(ch))
                  act( "$c0015[$c0005$n$c0015] dice '$N, dammi l'Anello "
                       "ADESSO'.", FALSE, ch, NULL, victim, TO_ROOM );
                else 
                {
                  act( "$n ti da` una dolorosa gomitata.",
                       FALSE, ch, NULL, victim, TO_ROOM);
                }
                wh->chances++;
              }
              break;
             default:
              if( check_peaceful( ch, "Dannazione, e` in salvo." ) )
              {
                if( !check_soundproof(ch) )
                  act( "$c0015[$c0005$n$c0015] dice 'Non puoi star qui per "
                       "sempre, $N'.", FALSE, ch, NULL, victim, TO_ROOM);
              } 
              else 
              {
                if (!check_soundproof(ch))
                  act( "$c0015[$c0005$n$c0015] dice 'Va bene, credo che "
                       "dovro` prendermelo da solo'.", FALSE, ch, NULL, 
                       victim, TO_ROOM);
                hit( ch, victim, TYPE_UNDEFINED );
              }
              break;
            }
          }
        } 
        else if( ring->in_obj ) 
        {
          /* the ring is in an object */
          obj_from_obj(ring);
          obj_to_char(ring, ch);
          act( "$n prende $p.", FALSE, ch, ring, NULL, TO_ROOM );
        } 
        else if( ring->in_room != NOWHERE )
        {
          obj_from_room( ring );
          obj_to_char( ring, ch );
          act( "$n prende $p.", FALSE, ch, ring, NULL, TO_ROOM );
        } 
        else 
        {
          mudlog( LOG_SYSERR, "a One Ring was completely disconnected!?" );
          wh->ringnumber = 0;
          return FALSE;
        }
        
        return TRUE;
      }
      else
      {      
        /* there aren't as many one rings in the game as we thought */
        howmanyrings = 0;
        get_obj_vis_world( ch, "999.primo anello.", &howmanyrings );
        wh->ringnumber = 0;
        ch->generic = 20;
      }
    }
  }
  else if( type == EVENT_DEATH )
  {
    if( ch->act_ptr )
      free( ch->act_ptr );
  }
  return FALSE;
}

int WarrenGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tch, *good;
  int max_good;
  
  if (cmd || !AWAKE(ch))
    return (FALSE);
  
  if (ch->specials.fighting) {
    fighter(ch, cmd, arg,mob,type);
    return(TRUE);
  }

  max_good = -1000;
  good = 0;
  
  if (check_peaceful(ch, ""))
    return FALSE;
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) > max_good) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_good = GET_ALIGNMENT(tch);
        good = tch;
      }
    }
  }
  
  if (good && (GET_ALIGNMENT(good->specials.fighting) <= 0)) {
    if (!check_soundproof(ch))
       act("$n screams 'DEATH TO GOODY-GOODIES!!!!'", 
           FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, good, TYPE_UNDEFINED);
    return(TRUE);
  }
  
  return(FALSE);
}



int zm_tired(struct char_data *zmaster)
{
  return GET_HIT(zmaster) < GET_MAX_HIT(zmaster)/2 ||
    GET_MANA(zmaster) < 40;
}

int zm_stunned_followers(struct char_data *zmaster)
{
  struct follow_type        *fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (GET_POS(fwr->follower)==POSITION_STUNNED)
      return TRUE;
  return FALSE;
}

void zm_init_combat(struct char_data *zmaster, struct char_data *target)
{
  struct follow_type        *fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (IS_AFFECTED(fwr->follower, AFF_CHARM) &&
        fwr->follower->specials.fighting==NULL &&
        fwr->follower->in_room == target->in_room) {
      if (GET_POS(fwr->follower) == POSITION_STANDING) {
        hit(fwr->follower, target, TYPE_UNDEFINED);
      } else if (GET_POS(fwr->follower)>POSITION_SLEEPING &&
                 GET_POS(fwr->follower)<POSITION_FIGHTING) {
        do_stand(fwr->follower, "", -1);
      }
    }
}

int zm_kill_fidos(struct char_data *zmaster)
{
  struct char_data        *fido_b;
  fido_b = find_mobile_here_with_spec_proc(fido, zmaster->in_room);
  if (fido_b) {
    if (!check_soundproof(zmaster)) {
       act("$n shrilly screams 'Kill that carrion beast!'", FALSE,
        zmaster, 0,0, TO_ROOM);
       zm_init_combat(zmaster, fido_b);
    }
    return TRUE;
  }
  return FALSE;
}

int zm_kill_aggressor(struct char_data *zmaster)
{
  struct follow_type        *fwr;
  if (zmaster->specials.fighting)
    {
      if (!check_soundproof(zmaster)) {
         act("$n bellows 'Kill that mortal that dares lay hands on me!'", 
             FALSE, zmaster, 0,0, TO_ROOM);
         zm_init_combat(zmaster, zmaster->specials.fighting);
         return TRUE;
       }
    }
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (fwr->follower->specials.fighting &&
        IS_AFFECTED(fwr->follower, AFF_CHARM))
      {
        if (!check_soundproof(zmaster)) {
          act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE,
            zmaster, 0,0, TO_ROOM);
          zm_init_combat(zmaster, fwr->follower->specials.fighting);
          return TRUE;
        }
      }
  return FALSE;
}

int zombie_master(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
#define ZM_MANA        10
#define ZM_NEMESIS 3060
{
  struct obj_data *temp1;
  struct char_data        *zmaster;
  int        dir;

  zmaster = find_mobile_here_with_spec_proc(zombie_master, ch->in_room);

  if (cmd!=0 || ch != zmaster || !AWAKE(ch))
    return FALSE;

  if (!check_peaceful(ch, "") &&
      (zm_kill_fidos(zmaster) || zm_kill_aggressor(zmaster))
      ) {
    do_stand(zmaster, "", -1);
    return TRUE;
  }

  switch (GET_POS(zmaster)) {
  case POSITION_RESTING:
    if (!zm_tired(zmaster))
      do_stand(zmaster, "", -1);
    break;
  case POSITION_SITTING:
    if (!zm_stunned_followers(zmaster)) {
      if (!check_soundproof(ch))
        act("$n says 'It took you long enough...'", FALSE,
          zmaster, 0, 0, TO_ROOM);
      do_stand(zmaster, "", -1);
    }
    break;
  case POSITION_STANDING:
    if (zm_tired(zmaster)) {
      do_rest(zmaster, "", -1);
      return TRUE;
    }

    temp1 = get_obj_in_list_vis(zmaster, "corpse",
                                real_roomp(zmaster->in_room)->contents);
    
    if (temp1) {
      if (GET_MANA(zmaster) < ZM_MANA) {
        if (1==dice(1,20))
          if (!check_soundproof(ch))
                act("$n says 'So many bodies, so little time' and sighs.",
              FALSE, zmaster, 0,0, TO_ROOM);
      } else {
        if (check_nomagic(ch, 0, 0))
          return(FALSE);

        if (!check_soundproof(ch)) {
          act("$n says 'Wonderful, another loyal follower!' and grins maniacly.",
            FALSE, zmaster, 0,0, TO_ROOM);
          GET_MANA(zmaster) -= ZM_MANA;
          spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
          if (zmaster->followers) {
            /* assume the new follower is top of the list? */
            AddHatred(zmaster->followers->follower,OP_VNUM,ZM_NEMESIS);
          }
        }
      }
      return TRUE;
    } else if (zm_stunned_followers(zmaster)) {
      do_sit(zmaster, "", -1);
      return TRUE;
    } else if (1==dice(1,20)) {
      act("$n searches for bodies.", FALSE, zmaster, 0,0, TO_ROOM);
      return TRUE;
    } else if ( 0<=(dir = find_path(zmaster->in_room, named_object_on_ground, (char *)("corpse"), -200, 0))) {
      go_direction(zmaster, dir);
      return TRUE;
    } else if (1==dice(1,5)) {
      act("$n can't find any bodies.", FALSE, zmaster, 0,0, TO_ROOM);
      return TRUE;
    } else {
      mobile_wander(zmaster);
    }
  }

  return FALSE;
}

int pet_shops(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;
  
  pet_room = ch->in_room+1;
  
  if (cmd==59) { /* List */
    send_to_char("Available pets are:\n\r", ch);
    for(pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\n\r", 24*GET_EXP(pet), pet->player.short_descr);
      send_to_char(buf, ch);
    }
    return(TRUE);
  } else if (cmd==56) { /* Buy */
    
    arg = one_argument(arg, buf);
    only_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */
    
    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n\r", ch);
      return(TRUE);
    }
    
    if (GET_GOLD(ch) < (GET_EXP(pet)*10)) {
      send_to_char("You don't have enough gold!\n\r", ch);
      return(TRUE);
    }
    
    GET_GOLD(ch) -= GET_EXP(pet)*10;
    
    pet = read_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(pet->specials.affected_by, AFF_CHARM);
    
    if (*pet_name) {
      sprintf(buf,"%s %s", pet->player.name, pet_name);
      free(pet->player.name);
      pet->player.name = (char*)strdup(buf);                
      
      sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
              pet->player.description, pet_name);
      free(pet->player.description);
      pet->player.description = (char *)strdup(buf);
    }
    
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);
    
    IS_CARRYING_W(pet) = 0;
    IS_CARRYING_N(pet) = 0;
    
    send_to_char("May you enjoy your pet.\n\r", ch);
    act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);
    
    return(TRUE);
  }
  
  /* All commands except list and buy */
  return(FALSE);
}

int Fountain(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{

  int bits, water;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *tmp_char;
  char container[20];           /* so we can be flexible */
  struct obj_data *obj;

  void name_to_drinkcon(struct obj_data *obj,int type);
  void name_from_drinkcon(struct obj_data *obj);

  
  if (cmd==248)
  { 
    /* fill */

    arg = one_argument(arg, buf); /* buf = object */
    bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &obj);
  
    if (!bits) return(FALSE);

    if (ITEM_TYPE(obj) !=ITEM_DRINKCON)
    {
      send_to_char("Questo non e` un contenitore per liquidi!\n\r", ch);
      return(TRUE);
    }
  
    if ((obj->obj_flags.value[2] != LIQ_WATER) && 
        (obj->obj_flags.value[1] != 0))
    {
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    }
    else
    {
      /* Calculate water it can contain */
      water = obj->obj_flags.value[0]-obj->obj_flags.value[1];

      if (water > 0)
      {
        obj->obj_flags.value[2] = LIQ_WATER;
        obj->obj_flags.value[1] += water;
        weight_change_object(obj, water);
        name_from_drinkcon(obj);
        name_to_drinkcon(obj, LIQ_WATER);
        act("$p e` pieno.", FALSE, ch,obj,0,TO_CHAR);
        act("$n riempie $p con acqua.", FALSE, ch,obj,0,TO_ROOM);
      }
    }
    return(TRUE);

  }
  else if (cmd==11)
  {
    /* drink */
    switch(ch->in_room)
    {
     case 13518:
     case 11014:
     case 5234:
     case 3141:
     case 13406:
      strncpy(container,"fountain",20);
      break;
     case 22642:
     case 22644:
     case 22646:
     case 22648:
      strncpy(container,"brook",20);
      break;
     default:
      strncpy(container,"fontana",20);
    };


    only_argument(arg,buf);

    if (str_cmp(buf, container) && str_cmp(buf, "acqua"))
    {
      return(FALSE);
    }
    
    sprintf(buf,"Stai bevendo dalla %s.\n\r",container);
    send_to_char(buf, ch);

    sprintf(buf,"$n beve dalla %s.",container);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);


    GET_COND(ch,THIRST) = 24;

    if(GET_COND(ch,THIRST)>20)
      act("Ti sei dissetato.",FALSE,ch,0,0,TO_CHAR);

    return(TRUE);
  }
  
  /* All commands except fill and drink */
  return(FALSE);
}

int bank (struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  
  static char buf[256];
  int money;
  
  money = atoi(arg);
  
  if (IS_NPC(ch))
    return(FALSE);

  save_char(ch, ch->in_room);

  if (GET_BANK(ch) > GetMaxLevel(ch)*40000 && GetMaxLevel(ch)<40) {
    send_to_char("I'm sorry, but we can no longer hold more than 40000 coins per level.\n\r", ch);
    GET_GOLD(ch) += GET_BANK(ch)-GetMaxLevel(ch)*40000;
    GET_BANK(ch) = GetMaxLevel(ch)*40000;
  }


  /*deposit*/
  if (cmd==219) {
    if (HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
      send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
      return(TRUE);
    }


    if (money > GET_GOLD(ch)) {
      send_to_char("You don't have enough for that!\n\r", ch);
      return(TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return(TRUE);
    } else if ((money + GET_BANK(ch) > GetMaxLevel(ch)*40000) &&
               (GetMaxLevel(ch)<40)){
      send_to_char("I'm sorry, Regulations only allow us to ensure 40000 coins per level.\n\r",ch);
      return(TRUE);
    } else {
      send_to_char("Thank you.\n\r",ch);
      GET_GOLD(ch) = GET_GOLD(ch) - money;
      GET_BANK(ch) = GET_BANK(ch) + money;
      sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      return(TRUE);
    }
    /*withdraw*/
  } else if (cmd==220) {

    if (HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
      send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
      return(TRUE);
    }


    if (money > GET_BANK(ch)) {
      send_to_char("You don't have enough in the bank for that!\n\r", ch);
      return(TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return(TRUE);
    } else {
      send_to_char("Thank you.\n\r",ch);
      GET_GOLD(ch) = GET_GOLD(ch) + money;
      GET_BANK(ch) = GET_BANK(ch) - money;
      sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      return(TRUE);
    }
  } else if (cmd == 221) {
    sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
    send_to_char(buf, ch);
    return(TRUE);
  }
  return(FALSE);
}



/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

int pray_for_items(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  char buf[256];
  int key_room, gold;
  bool found;
  struct obj_data *tmp_obj, *obj;
  struct extra_descr_data *ext;
  
  if (cmd != 176) /* You must pray to get the stuff */
    return FALSE;
  
  key_room = 1+ch->in_room;
  
  strcpy(buf, "item_for_");
  strcat(buf, GET_NAME(ch));
  
  gold = 0;
  found = FALSE;
  
  for( tmp_obj = real_roomp(key_room)->contents; tmp_obj; 
       tmp_obj = tmp_obj->next_content )
  {
    
    for(ext = tmp_obj->ex_description; ext; ext = ext->next)
    {
      if( ext->nMagicNumber == EXDESC_VALID_MAGIC )
      {
        if (str_cmp(buf, ext->keyword) == 0)
        {
          if (gold == 0)
          {
            gold = 1;
            act("$n kneels and at the altar and chants a prayer to Odin.",
                FALSE, ch, 0, 0, TO_ROOM);
            act("You notice a faint light in Odin's eye.",
                FALSE, ch, 0, 0, TO_CHAR);
          }
          obj = read_object(tmp_obj->item_number, REAL);
          obj_to_room(obj, ch->in_room);
          act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM);
          act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR);
          gold += obj->obj_flags.cost;
          found = TRUE;
        }
      }
    }
  }
  
  
  if (found)
  {
    GET_GOLD(ch) -= gold;
    GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
    return TRUE;
  }
  
  return FALSE;
}


/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */



#define CHAL_ACT \
"You are torn out of reality!\n\r\
You roll and tumble through endless voids for what seems like eternity...\n\r\
\n\r\
After a time, a new reality comes into focus... you are elsewhere.\n\r"


int chalice(struct char_data *ch, int cmd, const char *arg)
{
  /* 222 is the normal chalice, 223 is chalice-on-altar */
  
  struct obj_data *chalice;
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  static int chl = -1, achl = -1;
  
  if (chl < 1)
    {
      chl = real_object(222);
      achl = real_object(223);
    }
  
  switch(cmd)
    {
    case 10:    /* get */
      if (!(chalice = get_obj_in_list_num(chl,
                                          real_roomp(ch->in_room)->contents))
          && CAN_SEE_OBJ(ch, chalice))
        if (!(chalice = get_obj_in_list_num(achl,
                                            real_roomp(ch->in_room)->contents)) && CAN_SEE_OBJ(ch, chalice))
          return(0);
      
      /* we found a chalice.. now try to get us */                        
      do_get(ch, arg, cmd);
      /* if got the altar one, switch her */
      if (chalice == get_obj_in_list_num(achl, ch->carrying))
        {
          extract_obj(chalice);
          chalice = read_object(chl, VIRTUAL);
          obj_to_char(chalice, ch);
        }
      return(1);
      break;
    case 67: /* put */
      if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
        return(0);
      
      argument_interpreter(arg, buf1, buf2);
      if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
        {
          extract_obj(chalice);
          chalice = read_object(achl, VIRTUAL);
          obj_to_room(chalice, ch->in_room);
          send_to_char("Ok.\n\r", ch);
        }
      return(1);
      break;
    case 176: /* pray */
      if (!(chalice = get_obj_in_list_num(achl,
                                          real_roomp(ch->in_room)->contents)))
        return(0);
      
      do_action(ch, arg, cmd);  /* pray */
      send_to_char(CHAL_ACT, ch);
      extract_obj(chalice);
      act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, 2500);   /* before the fiery gates */
      do_look(ch, "", 15);
      return(1);
      break;
    default:
      return(0);
      break;
    }
}




int kings_hall(struct char_data *ch, int cmd, const char *arg)
{
  if (cmd != 176)
    return(0);
  
  do_action(ch, arg, 176);
  
  send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
  send_to_char(CHAL_ACT, ch);
  act("$n is struck by an intense beam of light and vanishes.",
      TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, 1420);  /* behind the altar */
  do_look(ch, "", 15);
  return(1);
}



/*
**  donation room
*/
int Donation(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type) 
{
  char check[40];

  if ((cmd != 10) && (cmd != 167)) {
    return(FALSE);
  }

  one_argument(arg, check);

  if (*check) {
    if (strncmp(check, "all", 3)==0) {
      send_to_char("Now now, that would be greedy!\n\r", ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

/*
  house routine for saved items.
*/

int House(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type) 
{
  char buf[100];
  struct obj_cost cost;
  int i, save_room;
  int count=0;

  if (IS_NPC(ch)) return(FALSE);
  
  /* if (cmd != rent) ignore */
  if (cmd != 92) {
    return(FALSE);
  } else {
    
    /*  verify the owner */
    if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name, 
                strlen(GET_NAME(ch)))) {
      send_to_char("Sorry, you'll have to find your own house.\n\r",ch);
      return(FALSE);
    }
    
    cost.total_cost = 0; /* Minimum cost */
    cost.no_carried = 0;
    cost.ok = TRUE; /* Use if any "-1" objects */
    
    add_obj_cost(ch, 0, ch->carrying, &cost);
    count = CountLims(ch->carrying);
    for(i = 0; i<MAX_WEAR; i++) {
       add_obj_cost(ch, 0, ch->equipment[i], &cost);
       count += CountLims(ch->equipment[i]);
    }
    
    if (!cost.ok) {
      return(FALSE);
    }

#if LIMITED_ITEMS

    if (count > MaxLimited(GetMaxLevel(ch))) {
      send_to_char("I'm sorry, but you to many limited items.\n\r", ch);
      return(FALSE);
    }

#endif

    sprintf(buf, "It will cost you %d coins per day\n\r", cost.total_cost);
    send_to_char(buf, ch);

    save_obj(ch, &cost,1);
    save_room = ch->in_room;

    if (ch->specials.start_room != 2)
      ch->specials.start_room = save_room;

    extract_char(ch);  /* CHARACTERS aren't freed by this */
    save_char(ch, save_room);
    ch->in_room = save_room;
  }
  return TRUE;
}

/***********************************************************************
                           SaveRoomContens
  Salva tutti gli oggetti presenti nella stanza se questa ha il flag
  SAVE_ROOM. Dovrebbe venir chiamata solo se i comandi sono CMD_GET,
  CMD_TAKE o CMD_DROP.
***********************************************************************/
int SaveRoomContens( struct char_data *pChar, int nCmd, char *pchArg, 
                     struct room_data *pRoom, int nEvent )
{
  return FALSE;
}

/***********************************************************************

                           CHESSBOARD PROCS

 ***********************************************************************/

#define SISYPHUS_MAX_LEVEL 15

/* This is the highest level of PC that can enter.  The highest level
   monster currently in the section is 14th.  It should require a fairly
   large party to sweep the section. */

int sisyphus( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
              int type )
{
  static int b=1;  /* use this as a switch, to avoid double challenges */
  
  if( cmd ) 
  {

    if( cmd <= 6 && cmd >= 1 && IS_PC( ch ) ) 
    {
      if (b) 
      {
        b = 0;
        do_look( mob, GET_NAME( ch ), 0 );
      } 
      else 
      {
        b = 1;
      }
      if( ( ch->in_room == Ivory_Gate ) && ( cmd == 4 ) ) 
      {
        if( ( SISYPHUS_MAX_LEVEL < GetMaxLevel( ch ) ) &&
            ( GetMaxLevel( ch ) < LOW_IMMORTAL ) )
        {
          if( !check_soundproof( ch ) ) 
          {
            act( "$n ti ferma minaccioso e dice: "
                 "'Se vuoi entrare dovrai passare sul mio corpo.'", TRUE,
                 mob, 0, ch, TO_VICT );
          }
          act( "Una smorfia cattiva di $n ti gela il sangue.", TRUE, mob, 0, 
               ch, TO_VICT );
          return(TRUE);
        }
      }
      return(FALSE);       
    } /* cmd 1 - 6 */

    /* doorbash */ 
    if (cmd == 258) 
    {
      act("$n dice: 'Non credo che ci riuscirai.'",FALSE,mob,0,ch,TO_VICT);
      return(TRUE);
    }

    return(FALSE);
  } 
  else 
  {
    if( ch->specials.fighting ) 
    {
      if( ( GET_POS( ch ) < POSITION_FIGHTING ) &&
          ( GET_POS( ch ) > POSITION_STUNNED ) )
      {
        StandUp(ch);
      } 
      else 
      {
        switch(number(1,10)) 
        {
        case 1: 
          do_say(ch, "heal", 0);
          break;

        case 2: 
          do_say(ch, "pzar", 0);
          break;

        default: 
          FighterMove( ch );
          break;
        }
      }
      return(FALSE);
    }
  }
  return(FALSE);
} /* end sisyphus */


int jabberwocky(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd)
    return(FALSE);
  
  if (ch->specials.fighting) 
  {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
        (GET_POS(ch) > POSITION_STUNNED)){
      StandUp( ch );
    } else {
      FighterMove(ch);
    }
    return(FALSE);
  }
  return FALSE;
}

int flame(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd)
    return(FALSE);
  if (ch->specials.fighting) 
  {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
        (GET_POS(ch) > POSITION_STUNNED))
    {
      StandUp(ch);
    } else {
      FighterMove(ch);
    }
    return(FALSE);
  }
  return FALSE;
}

int banana(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (!cmd) return(FALSE);
  
  if ((cmd >= 1) && (cmd <= 6) &&
      (GET_POS(ch) == POSITION_STANDING) &&
      (!IS_NPC(ch))) {
    if (!saves_spell(ch, SAVING_PARA)) {
      act("$N tries to leave, but slips on a banana and falls.",
          TRUE, ch, 0, ch, TO_NOTVICT);
      act("As you try to leave, you slip on a banana.",
          TRUE, ch, 0, ch, TO_VICT);
      GET_POS(ch) = POSITION_SITTING;
      return(TRUE); /* stuck */
    }
    return(FALSE);        /* he got away */
  }
  return(FALSE);
}

int paramedics(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict, *most_hurt;
  
  if (!cmd) {
    if (ch->specials.fighting) {
      return(cleric(ch, 0, "",mob,type));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {
        
        /* Find a dude to do good things upon ! */
        
        most_hurt = real_roomp(ch->in_room)->people;
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room ) {
          if (((float)GET_HIT(vict)/(float)hit_limit(vict) <
               (float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
              && (CAN_SEE(ch, vict)))
            most_hurt = vict;
        }
        if (!most_hurt) return(FALSE); /* nobody here */
        
        if ((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
            0.66) {
          if (number(0,5)==0) {
            act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
          }
          return TRUE;        /* not hurt enough */
        }

        if (!check_soundproof(ch)) {
          if(number(0,4)==0) {
            if (most_hurt != ch) {
              act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
              act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
            }          
            if (check_nomagic(ch, 0, 0))
              return(FALSE);

            act("$n utters the words 'judicandus dies' in unison.", 
                1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", 
                            SPELL_TYPE_SPELL, most_hurt, 0);
            return(TRUE);
          }
        }
      } else {/* I'm asleep or sitting */
        return(FALSE);
      }
    }
  }
  return(FALSE);
}

int jugglernaut(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct obj_data *tmp_obj;
  int i, j;
  
  if (cmd) return(FALSE);
  
  if (GET_POS(ch) == POSITION_STANDING) {
    
    if (random()%3) return FALSE;
    
    /* juggle something */
    
    if (IS_CARRYING_N(ch) < 1) return FALSE;
    
    i = random()%IS_CARRYING_N(ch);
    j = 0;
    for (tmp_obj = ch->carrying; (tmp_obj) && (j < i); j++) {
      tmp_obj = tmp_obj->next_content;
    }
    
    if (random()%6) {
      if (random()%2) {
        act("$n tosses $p high into the air and catches it.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      }  else {
        act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      }
    } else {
      act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      obj_from_char(tmp_obj);
      obj_to_room(tmp_obj, ch->in_room);
    }
    return(TRUE);        /* don't move, I dropped something */
  } else  { /* I'm asleep or sitting */
    return FALSE;
  }
  return(FALSE);
}
#if 0
static char *elf_comm[] = {
  "wake", "yawn",
  "stand", "say Well, back to work.", "get all",
  "eat bread", "wink",
  "w", "w", "s", "s", "s", "d", "open gate", "e",  /* home to gate*/
  "close gate",
  "e", "e", "e", "e", "n", "w", "n", /* gate to baker */
  "give all.bread baker", /* pretend to give a bread */
  "give all.pastry baker", /* pretend to give a pastry */
  "say That'll be 33 coins, please.",
  "echo The baker gives some coins to the Elf",
  "wave",
  "s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry", 
  "w", "s", "s", /* to main square */
  "s", "w", "w", "w", "w", /* back to gate */
  "pat sisyphus",
  "open gate", "w", "close gate", "u", "n", "n", "n", "e", "e", /* to home */
  "say Whew, I'm exhausted.", "rest", "$"
};
#endif

int delivery_elf(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
#define ELF_INIT     0
#define ELF_RESTING  1
#define ELF_GETTING  2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER   5
#define ELF_RETURN_HOME    6
  
  if (cmd) return(FALSE);

  return(FALSE);
  
  if (ch->specials.fighting)
    return FALSE;
  
  switch(ch->generic) {
    
  case ELF_INIT:
    if (ch->in_room == 0) {
      /* he has been banished to the Void */
    } else if (ch->in_room != Elf_Home) {
      if (GET_POS(ch) == POSITION_SLEEPING) {
        do_wake(ch, "", 0);
        do_stand(ch, "", 0);
      }
      do_say(ch, "Woah! How did i get here!", 0);
      do_emote(ch, "waves his arm, and vanishes!", 0);
      char_from_room(ch);
      char_to_room(ch, Elf_Home);
      do_emote(ch, "arrives with a Bamf!", 0);
      do_emote(ch, "yawns", 0);
      do_sleep(ch, "", 0);
      ch->generic = ELF_RESTING;
    } else {
      ch->generic = ELF_RESTING;
    }
    return(FALSE);
    break;
  case ELF_RESTING:
    {
      if ((time_info.hours > 6) && (time_info.hours < 9)) {
        do_wake(ch, "", 0);
        do_stand(ch, "", 0);
        ch->generic = ELF_GETTING;
      }
      return(FALSE);
    } break;
    
  case ELF_GETTING:
    {
      do_get(ch, "all.loaf", 0);
      do_get(ch, "all.biscuit", 0);
      ch->generic = ELF_DELIVERY;
      return(FALSE);
    } break;
  case ELF_DELIVERY:
    {
      if (ch->in_room != Bakery) {
        int        dir;
        dir = choose_exit_global(ch->in_room, Bakery, -100);
        if (dir<0) {
          ch->generic = ELF_INIT;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        do_give(ch, "6*biscuit baker", 0);
        do_give(ch, "6*loaf baker", 0);
        do_say(ch, "That'll be 33 coins, please.", 0);
        ch->generic = ELF_DUMP;
      }
      return(FALSE);
    } break;
  case ELF_DUMP:
    {
      if (ch->in_room != Dump)   {
        int        dir;
        dir = choose_exit_global(ch->in_room, Dump, -100);
        if (dir<0) {
          ch->generic = ELF_INIT;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        do_drop(ch, "10*biscuit", 0);
        do_drop(ch, "10*loaf", 0);
        ch->generic = ELF_RETURN_TOWER;
      }
      return(FALSE);
    } break;
  case ELF_RETURN_TOWER:
    {
      if (ch->in_room != Ivory_Gate)   {
        int        dir;
        dir = choose_exit_global(ch->in_room, Ivory_Gate, -200);
        if (dir<0) {
          ch->generic = ELF_INIT;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        ch->generic = ELF_RETURN_HOME;
      }         
      return(FALSE);
    }
    break;
  case ELF_RETURN_HOME:
    if (ch->in_room != Elf_Home)   {
      int        dir;
      dir = choose_exit_global(ch->in_room, Elf_Home, -200);
      if (dir<0) {
        ch->generic = ELF_INIT;
        return(FALSE);
      } else {
        go_direction(ch, dir);
      }
    } else {
      if (time_info.hours > 21) {
        do_say(ch, "Done at last!", 0);
        do_sleep(ch, "", 0);
        ch->generic = ELF_RESTING;
      } else {
        do_say(ch, "An elf's work is never done.", 0);
        ch->generic = ELF_GETTING;
      }
    }
    return(FALSE);
    break;
  default:
    ch->generic = ELF_INIT;
    return(FALSE);
  }
}



int delivery_beast(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct obj_data *o;
  
  if (cmd) return(FALSE);

  return(FALSE);
  
  if (time_info.hours == 6) {
    do_drop(ch, "all.loaf",0);
    do_drop(ch, "all.biscuit", 0);
  } else if (time_info.hours < 2) {
    if (!number(0,1)) {
      o = read_object(3012, VIRTUAL);
      obj_to_char(o, ch);
    } else {
      o = read_object(3013, VIRTUAL);
      obj_to_char(o, ch);
    }
  } else {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      do_sleep(ch, "", 0);
    }
  }
}

int Keftab(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  int found, targ_item;
  struct char_data *i;
  
  if (cmd)
    return(FALSE);
  
  if (!ch->specials.hunting) 
  {
    /* find a victim */
    
    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(ch)) {
        targ_item = SWORD_ANCIENTS;
        found = FALSE;
        while (!found) {        
          if ((HasObject(i, targ_item))&&(GetMaxLevel(i) < 30)) {
            AddHated(ch, i);
            SetHunting(ch, i);
            return(TRUE);
          } else {
            targ_item++;
            if (targ_item > SWORD_ANCIENTS+20)
              found = TRUE;
          }
        }
      }
    }      
  }
  else 
  {
    
    /* check to make sure that the victim still has an item */
    found = FALSE;
    targ_item = SWORD_ANCIENTS;
    while (!found) 
    {
      if (HasObject(ch->specials.hunting, targ_item)) 
      {
        return(FALSE);
      }
      else 
      {
        targ_item++;
        if( targ_item > SWORD_ANCIENTS+20 )
          found = TRUE;
      }
    }
    ch->specials.hunting = 0;
  }
  return FALSE;
}

int StormGiant(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;
  
  if (cmd) 
    return(FALSE);
  
  if (ch->specials.fighting) 
  {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
        (GET_POS(ch) > POSITION_STUNNED)){
      StandUp(ch);
    } else {
      if (number(0, 5)) {
        fighter(ch, cmd, arg,mob,type);
      } else {
        act("$n creates a lightning bolt", TRUE, ch, 0,0,TO_ROOM);
        if ((vict = FindAHatee(ch)) == NULL)
              vict = FindVictim(ch);
        if (!vict) return(FALSE);
               cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        
        /* do nothing */
      }
    }
  }
  return(FALSE);
}

int Manticore(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(FALSE);
}

int Kraken(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(FALSE);
}


int fighter(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
    
  if (ch->specials.fighting)
  {
    if (GET_POS(ch) == POSITION_FIGHTING)
    {
      FighterMove(ch);
    }
    else
    {
      StandUp(ch);
    }
    FindABetterWeapon(ch);
  }
  return(FALSE);
}





/*
**  NEW THALOS MOBS:******************************************************
*/


#define NTMOFFICE  13554
#define NTMNGATE   13422
#define NTMEGATE   13431
#define NTMSGATE   13413
#define NTMWGATE   13423

#define NTMWMORN    0
#define NTMSTARTM   1
#define NTMGOALNM   2
#define NTMGOALEM   3
#define NTMGOALSM   4
#define NTMGOALWM   5
#define NTMGOALOM   6
#define NTMWNIGHT   7 
#define NTMSTARTN   8
#define NTMGOALNN   9
#define NTMGOALEN   10
#define NTMGOALSN   11
#define NTMGOALWN   12
#define NTMGOALON   13
#define NTMSUSP     14
#define NTM_FIX     15

int NewThalosMayor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  
  if (cmd || !AWAKE(ch)) 
    return(FALSE);
  
  if (ch->specials.fighting) 
  {
    return(FALSE);
  }
  else 
  {
    switch(ch->generic) 
    {  /* state info */
    case NTMWMORN:  /* wait for morning */
      if (time_info.hours == 6) 
      {
        ch->generic = NTMGOALNM;
        return(FALSE);
      }
      break;
    case NTMGOALNM: /* north gate */       
    {
      if (ch->in_room != NTMNGATE) 
      {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
        if (dir<0) 
        {
          ch->generic = NTM_FIX;
          return(FALSE);
        } 
        else 
        {
          go_direction(ch, dir);
        }
      } 
      else 
      {
        /*
         * unlock and open door.
         */
        do_unlock(ch, " gate", 0);
        do_open(ch, " gate", 0);
        ch->generic = NTMGOALEM;
      }
      return(FALSE);
    }
    break;
    case NTMGOALEM:       
    {
      if (ch->in_room != NTMEGATE) 
      {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
        if (dir<0) 
        {
          ch->generic = NTM_FIX;
          return(FALSE);
        } 
        else 
        {
          go_direction(ch, dir);
        }
      } 
      else 
      {
        /*
         * unlock and open door.
         */
        do_unlock(ch, " gate", 0);
        do_open(ch, " gate", 0);
        ch->generic = NTMGOALSM;
      }
      return(FALSE);
    }
    case NTMGOALSM:       
    {
      if (ch->in_room != NTMSGATE) 
      {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
        if (dir<0) 
        {
          ch->generic = NTM_FIX;
          return(FALSE);
        } 
        else 
        {
          go_direction(ch, dir);
        }
      } 
      else 
      {
        /*
         * unlock and open door.
         */
        do_unlock(ch, " gate", 0);
        do_open(ch, " gate", 0);
        ch->generic = NTMGOALWM;
      }
      return(FALSE);
    }
    case NTMGOALWM:       
    {
      if (ch->in_room != NTMWGATE) 
      {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
        if (dir<0) 
        {
          ch->generic = NTM_FIX;
          return(FALSE);
        } 
        else 
        {
          go_direction(ch, dir);
        }
      } 
      else 
      {
        /*
         * unlock and open door.
         */
        do_unlock(ch, " gate", 0);
        do_open(ch, " gate", 0);
        ch->generic = NTMGOALOM;
      }
      return(FALSE);
    }
    case NTMGOALOM:       
    {
      if (ch->in_room != NTMOFFICE) {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
        if (dir<0) {
          ch->generic = NTM_FIX;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        ch->generic = NTMWNIGHT;
      }
      return(FALSE);
    }
    case NTMWNIGHT:  /* go back to wait for 7pm */
      if (time_info.hours == 19) {
        ch->generic = NTMGOALNN;
      }
    case NTMGOALNN: /* north gate */       {
      if (ch->in_room != NTMNGATE) {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
        if (dir<0) {
          ch->generic = NTM_FIX;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        /*
         * lock and open door.
         */
        do_lock(ch, " gate", 0);
        do_close(ch, " gate", 0);
        ch->generic = NTMGOALEN;
      }
      return(FALSE);
    }
    case NTMGOALEN:       {
      if (ch->in_room != NTMEGATE) {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
        if (dir<0) {
          ch->generic = NTM_FIX;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        /*
         * lock and open door.
         */
        do_lock(ch, " gate", 0);
        do_close(ch, " gate", 0);
        ch->generic = NTMGOALSN;
      }
      return(FALSE);
    }
    case NTMGOALSN:       {
      if (ch->in_room != NTMSGATE) {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
        if (dir<0) {
          ch->generic = NTM_FIX;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        /*
         * lock and open door.
         */
        do_lock(ch, " gate", 0);
        do_close(ch, " gate", 0);
        ch->generic = NTMGOALWN;
      }
      return(FALSE);
    }
    case NTMGOALWN:       {
      if (ch->in_room != NTMWGATE) {
        int        dir;
        dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
        if (dir<0) {
          ch->generic = NTM_FIX;
          return(FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        /*
         * unlock and open door.
         */
        do_lock(ch, " gate", 0);
        do_close(ch, " gate", 0);
        ch->generic = NTMGOALOM;
      }
      return(FALSE);
    }
    case NTMGOALON:      {
        if (ch->in_room != NTMOFFICE) {
          int        dir;
          dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
          if (dir<0) {
            ch->generic = NTM_FIX;
            return(FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          ch->generic = NTMWMORN;
        }
        return(FALSE);
        break;
      }
    case NTM_FIX: {
    /*
     * move to correct spot (office)
     */
      do_say(ch, "Woah! How did i get here!", 0);
      char_from_room(ch);
      char_to_room(ch, NTMOFFICE);
      ch->generic = NTMWMORN;
      return(FALSE);
      break;
    }
    default: {
      ch->generic = NTM_FIX;
      return(FALSE);
      break;
      }
    }
  }
  return FALSE;
}

int SultanGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(GenericCityguard(ch,cmd,arg,mob,NEWTHALOS));
}


int NewThalosCitizen(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (ch->specials.fighting) 
  {
    fighter(ch, cmd, arg, mob, type);

    if (!check_soundproof(ch)) 
    {

       if (number(0,18) == 0) {
         do_shout(ch, "Guards! Help me! Please!", 0);
       } else {
         act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
       }
    
       if (ch->specials.fighting)
         CallForGuard(ch, ch->specials.fighting, 3, NEWTHALOS);
    
       return(TRUE);
     }
  } 
  return(FALSE);
}

int NewThalosGuildGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (!cmd) {
     if (ch->specials.fighting) {
       return(fighter(ch, cmd, arg, mob, type));
     }
  } else {
     if (cmd >= 1 && cmd <= 6) { 
       switch(ch->in_room) {
       case 13532:
          return(CheckForBlockedMove(ch,cmd,arg,13532,2,CLASS_THIEF));
          break;
       case 13512:
          return(CheckForBlockedMove(ch, cmd, arg, 13512, 2, CLASS_CLERIC));
          break;
       case 13526:
          return(CheckForBlockedMove(ch,cmd, arg, 13526, 2, CLASS_WARRIOR));
          break;
       case 13525:
          return(CheckForBlockedMove(
                  ch, cmd, arg, 13525,0, CLASS_MAGIC_USER|CLASS_SORCERER));
          break;
      }
    }
  }
  return(FALSE);
}

/*
 * New improved magic_user
 */


int magic_user2(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;
  byte lspell;
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
  
  if (!ch->specials.fighting) return FALSE;
  
  if ((GET_POS(ch) > POSITION_STUNNED) &&
      (GET_POS(ch) < POSITION_FIGHTING))
  {
    StandUp(ch);
    return(TRUE);
  }

  if (check_nomagic(ch, 0, 0))
    return(FALSE);

  vict = ch->specials.fighting;
  
  if (!vict) 
    vict = FindVictim(ch);
  
  if (!vict)
    return(FALSE);
  
  lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */
  
  if (lspell < 1) lspell = 1;
  
  if ((vict!=ch->specials.fighting) && (lspell>19) && (!IS_AFFECTED(ch, AFF_FIRESHIELD)))
      {
    act("$n utters the words 'Fireshield'.", 1, ch, 0, 0, TO_ROOM);
    cast_fireshield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return TRUE;
  }

  switch (lspell)
      {
      case 1:
    act("$n utters the words 'Magic Missile'.", 1, ch, 0, 0, TO_ROOM);
    cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 2:
    act("$n utters the words 'Shocking Grasp'.", 1, ch, 0, 0, TO_ROOM);
    cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 3:
  case 4:
    act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
    cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 5:
    act("$n utters the words 'Burning Hands'.", 1, ch, 0, 0, TO_ROOM);
    cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 6:
  if (!IS_AFFECTED(vict, AFF_SANCTUARY))
    {
    act("$n utters the words 'Dispel Magic'.", 1, ch, 0, 0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
  }
  else
    {
    act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
    cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
  }
    break;
  case 7:
    act("$n utters the words 'Ice Storm'.", 1, ch, 0, 0, TO_ROOM);
    cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 8:
    act("$n utters the words 'Blindness'.", 1, ch, 0, 0, TO_ROOM);
    cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 9:
    act("$n utters the words 'Fear'.", 1, ch, 0, 0, TO_ROOM);
    cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 10:
  case 11:
    act("$n utters the words 'Lightning Bolt'.", 1, ch, 0, 0, TO_ROOM);
    cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 12:
  case 13:
    act("$n utters the words 'Color Spray'.", 1, ch, 0, 0, TO_ROOM);
    cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 14:
    act("$n utters the words 'Cone Of Cold'.", 1, ch, 0, 0, TO_ROOM);
    cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    act("$n utters the words 'Fireball'.", 1, ch, 0, 0, TO_ROOM);
    cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;

  default:
    act("$n utters the words 'frag'.", 1, ch,0,0,TO_ROOM);
    cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
    
  }
  
  return TRUE;
  
}


/******************Mordilnia citizens************************************/

int MordGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(GenericCityguardHateUndead(ch,cmd,arg,mob,MORDILNIA));
}

int MordGuildGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (!cmd) {
     if (ch->specials.fighting) {
       return(fighter(ch, cmd, arg, mob, type));
     }
  } else {
     if (cmd >= 1 && cmd <= 6) { 
       switch(ch->in_room) {
       case 18266:
          return(CheckForBlockedMove(
                 ch,cmd,arg,18266,2,CLASS_MAGIC_USER|CLASS_SORCERER));
          break;
       case 18276:
          return(CheckForBlockedMove(ch, cmd, arg, 18276, 2, CLASS_CLERIC));
          break;
       case 18272:
          return(CheckForBlockedMove(ch, cmd, arg, 18272, 2, CLASS_THIEF));
          break;
       case 18256:
          return(CheckForBlockedMove(ch, cmd, arg, 18256, 0, CLASS_WARRIOR));
          break;
      }
     } else {
       return(FALSE);
     }
  }
  return(FALSE);
}


int CaravanGuildGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (!cmd) {
     if (ch->specials.fighting) {
       return(fighter(ch, cmd, arg, mob, type));
     }
  } else {
     if (cmd >= 1 && cmd <= 6) { 
       switch(ch->in_room) {
       case 16115:
          return(CheckForBlockedMove(
                 ch,cmd,arg,16115,1,CLASS_MAGIC_USER|CLASS_SORCERER));
          break;
       case 16126:
          return(CheckForBlockedMove(ch, cmd, arg, 16116, 1, CLASS_CLERIC));
          break;
       case 16117:
          return(CheckForBlockedMove(ch, cmd, arg, 16117, 3, CLASS_THIEF));
          break;
       case 16110:
          return(CheckForBlockedMove(ch, cmd, arg, 16110, 3, CLASS_WARRIOR));
          break;
      }
     } else {
       return(FALSE);
     }
  }
  return(FALSE);
}

int StatTeller(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  int choice;
  char buf[200];

  if (cmd) {
    if (cmd == 56) { /* buy */

  /*
  ** randomly tells a player 3 of his/her stats.. for a price
  */
      if (GET_GOLD(ch)< 1000) {
        send_to_char("You do not have the money to pay me.\n\r", ch);
        return(TRUE);
      } else {
        GET_GOLD(ch)-=1000;
      }

      choice = number(0,2);
      switch(choice) {
      case 0:
        sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r", GET_STR(ch), GET_WIS(ch), GET_DEX(ch));
        send_to_char(buf, ch);
        break;
      case 1:
        sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r", GET_INT(ch), GET_DEX(ch), GET_CON(ch));
        send_to_char(buf, ch);
        break;
      case 2:
        sprintf(buf, "CON: %d, INT: %d , WIS: %d \n\r", GET_CON(ch), GET_INT(ch), GET_WIS(ch));
        send_to_char(buf, ch);
        break;
      default:
        send_to_char("We are experiencing Technical difficulties\n\r", ch);
        return(TRUE);
      }

    } else {
      return(FALSE);
    }
  }else {    

  /*
  **  in combat, issues a more potent curse.
  */

    if (ch->specials.fighting) {
      act("$n gives you the evil eye!  You feel your hitpoints ebbing away", 
          FALSE, ch, 0, ch->specials.fighting, TO_VICT);
      act("$n gives $N the evil eye!  $N seems weaker!", 
          FALSE, ch, 0, ch->specials.fighting, TO_NOTVICT);
      ch->specials.fighting->points.max_hit -= 10;
      ch->specials.fighting->points.hit -= 10;
      return(FALSE);
    }

  }
  return(FALSE);
}

void ThrowChar(struct char_data *ch, struct char_data *v, int dir)
{
  struct room_data *rp;
  int orr;
  char buf[200];

  rp = real_roomp(v->in_room);
  if (rp && rp->dir_option[dir] &&
      rp->dir_option[dir]->to_room && 
      (EXIT(v, dir)->to_room != NOWHERE)) {
    if (v->specials.fighting) {
      stop_fighting(v);
    }
    sprintf(buf, "%s picks you up and throws you %s\n\r", 
            ch->player.short_descr, dirs[dir]);
    send_to_char(buf,v);
    orr = v->in_room;
    char_from_room(v);
    char_to_room(v,(real_roomp(orr))->dir_option[dir]->to_room);
    do_look(v, "\0",15);
    
    if (IS_SET(RM_FLAGS(v->in_room), DEATH) && 
        GetMaxLevel(v) < LOW_IMMORTAL) {
      NailThisSucker(v);
    }
  }
}

int ThrowerMob(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;

   /*
   **  Throws people in various directions
   */

  if (!cmd) {
    if (AWAKE(ch) && ch->specials.fighting) {
      /*
      **  take this person and throw them
      */
      vict = ch->specials.fighting;
      switch(ch->in_room) {
      case 13912:
        ThrowChar(ch, vict, 1);  /* throw chars to the east */
        return(FALSE);
        break;
      default:
        return(FALSE);
      }
    }
  } else {
    switch(ch->in_room) {
    case 13912: {
      if (cmd == 1) {   /* north+1 */
          send_to_char("The Troll blocks your way.\n",ch);
        return(TRUE);
      }
      break;
    }
    default:
      return(FALSE);
    }
  }
  return(FALSE);
}



#if 0
/*
Smart thief special
*/

Thief(struct char_data *ch, const char *arg, ind cmd, struct char_data *mob, int type)
{

  if (cmd || !AWAKE(ch)) return;

}

#endif



/*
Swallower special
*/

int Tyrannosaurus_swallower(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct obj_data *co, *o;
  struct char_data *targ;
  struct room_data *rp;
  int i;
  //extern char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  extern char DestroyedItems;

  if (cmd && cmd != 156) return(FALSE);

  if( cmd == 156 )
  {
    send_to_char("You're much too afraid to steal anything!\n\r", ch);
    return(TRUE);
  }

/*
** damage stuff
*/
  DestroyedItems = 0;

  DamageStuff( ch, SPELL_ACID_BLAST, 100 );

  if( DestroyedItems )
  {
    act("$n lets off a real rip-roarer!", FALSE, ch, 0, 0, TO_ROOM);
    DestroyedItems = 0;
  }

/*
**  swallow
*/

  if( AWAKE(ch) )
  {
    if( ( targ = FindAnAttacker(ch) ) != NULL )
    {
      act( "$n opens $s gaping mouth", TRUE, ch, 0, 0, TO_ROOM );
      if( !CAN_SEE(ch, targ) )
      {
        if( saves_spell(targ, SAVING_PARA) )
        {
          act("$N barely misses being swallowed whole!", 
              FALSE, ch, 0, targ, TO_NOTVICT);
          act("You barely avoid being swallowed whole!", 
              FALSE, ch, 0, targ, TO_VICT);
          return(FALSE);
        }
      }
      if (!saves_spell(targ, SAVING_PARA)) 
      {
        act("In a single gulp, $N is swallowed whole!", 
            TRUE, ch, 0, targ, TO_ROOM);
        send_to_char("In a single gulp, you are swallowed whole!\n\r", targ);
        send_to_char("The horror!  The horror!\n\r", targ);
        send_to_char("MMM.  yum!\n\r", ch);
        /* kill target: */
        GET_HIT(targ) = 0;
        mudlog( LOG_PLAYERS, "%s killed by being swallowed whole", 
                GET_NAME( targ ) );
        die( targ, 0 );
        /*
          all stuff to monster:  this one is tricky.  assume that corpse is
          top item on item_list now that corpse has been made.
          */
        rp = real_roomp(ch->in_room);
        if (!rp) 
          return(FALSE);
        for (co = rp->contents; co; co = co->next_content) 
        {
          if (IS_CORPSE(co))  
          {  /* assume 1st corpse is victim's */
            while (co->contains) 
            {
              o = co->contains;
              obj_from_obj(o);
              obj_to_char(o, ch);
              if (ITEM_TYPE(o) == ITEM_POTION) 
              {
                /* do the effects of the potion: */
                for (i=1; i<4; i++)
                  if (o->obj_flags.value[i] >= 1)
                    ((*spell_info[o->obj_flags.value[i]].spell_pointer)
                     ((byte) o->obj_flags.value[0], ch, "", 
                      SPELL_TYPE_POTION, ch, o));
                extract_obj(o);
                
              }
            }
            extract_obj(co);  /* remove the corpse */
            return(TRUE);
          }
        }
      } 
      else 
      {
        act("$N barely misses being swallowed whole!", FALSE, ch, 0, targ, TO_NOTVICT);
        act("You barely avoid being swallowed whole!", FALSE, ch, 0, targ, TO_VICT);
      }
    }
  }
  return FALSE;
}




int soap( struct char_data *ch, int cmd, const char *arg, struct obj_data *tobj,
          int type)
{
  struct char_data *t;
  struct obj_data *obj;
  char dummy[80], name[80];
  int (*wash)( struct char_data *, int, const char *, struct obj_data *, int );

  wash = soap;

  if( type != EVENT_COMMAND )
    return(FALSE);
  
  if( cmd != 172 )
    return(FALSE);
  
  if (!(obj = ch->equipment[HOLD]))
    return(FALSE);
  if( obj->item_number < 0 || obj_index[obj->item_number].func != CASTVF wash )
    return(FALSE);

  arg = one_argument(arg, dummy);
  if(!(*dummy)) 
    return(FALSE);
  only_argument(arg, name);
  if(!(*name)) 
    return(FALSE);
  
  t = get_char_room_vis( ch, name );
  if( !t )
    return( FALSE );

  if (affected_by_spell(t,SPELL_WEB)) 
  {
    affect_from_char(t,SPELL_WEB);
    act("$n washes some webbing off $N with $p.",TRUE,ch,obj,t,TO_ROOM);
    act("You wash some webbing off $N with $p.",FALSE,ch,obj,t,TO_CHAR);
  }
  else 
  {
    act("$n gives $N a good lathering with $p.",TRUE,ch,obj,t,TO_ROOM);
    act("You give $N a good lathering with $p.",FALSE,ch,obj,t,TO_CHAR);
  }
  
  obj->obj_flags.value[0]--;
  if(!obj->obj_flags.value[0]) 
  {
    act("That used up $p.",FALSE,ch,obj,t,TO_CHAR);
    extract_obj(obj);
  }
  return TRUE;
}  

int nodrop(struct char_data *ch, int cmd, const char *arg, struct obj_data *tobj, int type)
{
  struct char_data *t;
  struct obj_data *obj, *i;
  char buf[80], obj_name[80], vict_name[80], *name;
  bool do_all;
  int j, num;
  int (*knowdrop)(struct char_data *, int, const char *, struct obj_data *, int );

  switch(cmd)
  {
    case 10: /* Get */
    case 60: /* Drop */
    case 72: /* Give */
    case 156: /* Steal */
    break;
    default:
    return(FALSE);
  }
  
  knowdrop = nodrop;

  if (type != EVENT_COMMAND)
    return(FALSE);
  
  arg = one_argument(arg, obj_name);
  if (!*obj_name) return(FALSE);

  obj = 0x0;
  do_all = FALSE;

  if(!(strncmp(obj_name,"all",3))) {
    do_all = TRUE;
    num = IS_CARRYING_N(ch);
  }
  else {
    strcpy(buf,obj_name);
    name = buf;
    if(!(num = get_number(&name))) return(FALSE);
  }

  /* Look in the room first, in get case */
  if(cmd == 10) {
    for (i=real_roomp(ch->in_room)->contents,j=1;i&&(j<=num);i=i->next_content) {
      if (i->item_number>=0) {
        if (do_all || isname(name, i->name)) {
          if(do_all || j == num) {
            if (obj_index[i->item_number].func == CASTVF knowdrop) {
              obj = i;
              break;
            }
          }
          else ++j;
        }
      }
    }
  }
  
  /* Check the character's inventory for give, drop, steal. */
  if(!obj) {
    /* Don't bother with get anymore */
    if(cmd == 10) return(FALSE);
    for (i = ch->carrying,j=1;i&&(j<=num);i=i->next_content) {
      if (i->item_number>=0) {
        if (do_all || isname(name, i->name)) {
          if(do_all || j == num) {
            if (obj_index[i->item_number].func == CASTVF knowdrop) {
              obj = i;
              break;
            }
            else if(!do_all) return(FALSE);
          }
          else ++j;
        }
      }
    }
  }
  
  /* Musta been something else */
  if(!obj)
    return(FALSE);
  
  if( cmd == 72 || cmd == 156 )
  {
    only_argument(arg, vict_name);
    if( !*vict_name )
      return FALSE;
    else if( !( t = get_char_room_vis( ch, vict_name ) ) )
      return FALSE;
  }
  else
    t = NULL; /* Non viene usato per gli altri comandi. */
  
  switch(cmd)
  {
   case 10:
    if(GetMaxLevel(ch)<=MAX_MORT)
    {
      act("$p disintegrates when you try to pick it up!",
          FALSE, ch, obj, 0, TO_CHAR);
      act("$n tries to get $p, but it disintegrates in his hand!",
          FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
      if(do_all) return(FALSE);
      else return(TRUE);
    }
    else return(FALSE);

    case 60:
    if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP))
    {
      act("You drop $p to the ground, and it shatters!",
          FALSE, ch, obj, 0, TO_CHAR);
      act("$n drops $p, and it shatters!", FALSE, ch, obj, 0, TO_ROOM);
      i = read_object(30, VIRTUAL);
      sprintf(buf, "Scraps from %s lie in a pile here.",
              obj->short_description);
      i->description = (char *)strdup(buf);
      obj_to_room(i, ch->in_room);
      obj_from_char(obj);
      extract_obj(obj);
      if(do_all) return(FALSE);
      else return(TRUE);
    }
    else return(FALSE);
    
    case 72:
    if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
      if(GetMaxLevel(ch)<=MAX_MORT) {
        act("You try to give $p to $N, but it vanishes!",
            FALSE, ch, obj, t, TO_CHAR);
        act("$N tries to give $p to you, but it fades away!",
            FALSE, t, obj, ch, TO_CHAR);
        act("As $n tries to give $p to $N, it vanishes!",
            FALSE, ch, obj, t, TO_ROOM);
        extract_obj(obj);
        if(do_all) return(FALSE);
        else return(TRUE);
      }
      else return(FALSE);
    }
    else return(FALSE);
    
    case 156: /* Steal */
    if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
      act("You cannot seem to steal $p from $N.",
          FALSE, ch, obj, t, TO_CHAR);
      act("$N tried to steal something from you!",FALSE,t,obj,ch,TO_CHAR);
      act("$N tried to steal something from $n!",FALSE,t,obj,ch,TO_ROOM);
      return(TRUE);
    }
    else return(FALSE);
    
    default:
    return(FALSE);
  }
  
  return(FALSE);
}

int andy_wilcox(char_data *, int, const char *, char_data *, int) {return FALSE;}
