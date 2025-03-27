NAME = ft_traceroute

#########
RM = rm -f
CC = cc
CFLAGS = -Werror -Wextra -Wall #-g -fsanitize=address
#########

#########
FILES = main ft_traceroute

SRC = $(addsuffix .c, $(FILES))

vpath %.c srcs
#########

#########
OBJ_DIR = objs
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
DEP = $(addsuffix .d, $(basename $(OBJ)))
#########

#########
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	${CC} $(CFLAGS) -MMD -c $< -o $@

all:
	$(MAKE) $(NAME) 

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -lm
	@echo "EVERYTHING DONE  "

clean:
	$(RM) $(OBJ) $(DEP) 
	$(RM) -r $(OBJ_DIR)
	@echo "OBJECTS REMOVED   "

fclean: clean
	$(RM) $(NAME) 
	@echo "EVERYTHING REMOVED   "

re:	fclean all

.PHONY: all clean fclean re 
	
-include $(DEP)

#########