# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: david <david@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/16 20:28:22 by guillaumeph       #+#    #+#              #
#    Updated: 2025/07/09 01:46:11 by david            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# **************************************************************************** #
#                                 Makefile Portable                            #
# **************************************************************************** #

NAME        := ircserv
SRC_DIR     := srcs
OBJ_DIR     := build
INC_DIR     := includes

SRCS        := \
	srcs/main.cpp \
	srcs/server/Server.cpp \
	srcs/client/Client.cpp \
	srcs/channel/Channel.cpp \
	srcs/server/CommandParser.cpp \
	srcs/server/ChannelManager.cpp \
	srcs/server/Utils.cpp
OBJS        := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# OS DETECTION (MAC / LINUX)
UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SYSROOT := -isysroot $(shell xcrun --show-sdk-path)
else
	SYSROOT :=
endif

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -I. -I$(INC_DIR) -Isrcs/includes -Isrcs/server/includes

# ----------------------------------------------------------------------------- #

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp #| $(OBJ_DIR)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#$(OBJ_DIR):
#	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

updateSources:
	@bash -c '\
		SEARCH_DIR="$(SRC_DIR)"; \
		IGNORED_DIRS=("dontcopy"); \
		FIND_CMD="find $$SEARCH_DIR -type f -name '\''*.cpp'\''"; \
		for dir in "$${IGNORED_DIRS[@]}"; do \
			FIND_CMD+=" ! -path '\''$$SEARCH_DIR/$$dir/*'\''"; \
		done; \
		eval $$FIND_CMD | sort | awk '\''{print "\t" $$0 " \\"}'\'' \
			| sed '\''$$ s/ \\$$//'\'' > .sources_block; \
		echo "SRCS        := \\" > .sources_full; \
		cat .sources_block >> .sources_full; \
		awk '\'' \
			FNR==NR { lines[NR] = $$0; next } \
			/^SRCS[[:space:]]*:=/ { \
				print lines[1]; \
				for (i = 2; i <= length(lines); ++i) print lines[i]; \
				in_block = 1; next; \
			} \
			in_block && /^[^[:space:]]/ { in_block = 0 } \
			!in_block { print } \
		'\'' .sources_full Makefile > Makefile.tmp; \
		mv Makefile.tmp Makefile; \
		rm -f .sources_block .sources_full; \
		echo "✅ Makefile mis à jour avec les fichiers sources."'

.PHONY: all clean fclean re updateSources
