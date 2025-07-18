#ifndef NUMERICS_HPP
# define NUMERICS_HPP

#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_MYINFO "004"
#define RPL_AWAY "301"
#define RPL_TOPIC "332"
#define RPL_INVITING "341"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"

// Section 5.2 Des cas d'erreur du site que
// Guillaume nous a drop a en add si besoin
#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_NOTEXTTOSEND "412"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NOTONCHANNEL "442"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_BADCHANMASK "476"
#define ERR_BADCHAR "479"
#define ERR_TOOMANYMASK "480"

#endif
