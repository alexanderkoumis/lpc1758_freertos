4
""" A python module to play a variant of the popular Connect Four game.

- Matthew Carlis
"""
import sys
import copy
import time
from game_socket import GameSocket
#import random as rand

bottom_row = "|1|2|3|4|5|6|7|"
empty_row = '|-|-|-|-|-|-|-|'
null_row = [0,0,0,0,0,0,0]

GAME_STATES = {'earthquake': 3, 'human_player': 2, 'ai_player': 1}
"""
I am thinking we can represent the board like this
0 if its empty and we can change the value to X or O
given the situation

"""
global_board = [[1,0,0,0,0,0,1], # 0  - 7 x infinity
                [0,1,0,0,0,1,0], # 1
                [0,0,1,0,1,0,0], # 2
                [0,0,1,0,1,0,0], # 3
                [0,1,0,0,0,1,0], # 4
                [1,0,0,0,0,0,1]] # 5
"""
My idea is to have each Node represent a state in the game.
"""

class GameBoard(object):
    """ A game board object.
    """
    def __init__(self, width=7, init_height=6, game_board=None):
        """ height_map: A variable to store the use height of the
                columns of the board.
            the_board: A list of lists which represents the board.
        """
        self.player_1, self.player_2 = 1, 2
        self.row_gen = lambda num_cols: [0 for val in range(num_cols)]
        if game_board is None:
            self.the_board = [self.row_gen(width) for row in range(init_height)]
            self.height_map = self.row_gen(width)
            # Store each players longest sequence of connections
            self.players_heuristics = {}
            self.width = width
            self.height = init_height
        else:
            self.the_board = copy.deepcopy(game_board.the_board)
            self.height_map = copy.deepcopy(game_board.height_map)
            # Store each players longest sequence of connected moves.
            self.players_heuristics = copy.deepcopy(game_board.players_heuristics)
            self.width = game_board.width
            self.height = game_board.height

    def __repr__(self):
        map_str = '['
        for cnt, row in enumerate(self.the_board):
            if cnt == 0:
                map_str += '{}\n'.format(row)
            elif cnt != len(self.the_board) - 1:
                map_str += ' {}\n'.format(row)
            else:
                map_str += ' {}]\n'.format(row)
        return map_str

    def __str__(self):
        return self.__repr__()

    def terminal_check(self):
        if sum(self.height_map) == self.width * self.height:
            return True
        return False

    def insert(self, column_num, value):
        """ column_number: The Zero left aligned column index to insert value
            value: The value to be inserted.
            Checks the number of connected <value> at this position to update
                the maximum occurences of consecutive <value>.
        """
        # Insert a new row if we're at the top of this column_num
        self.last_col_ins = column_num
        self.last_val_ins = value
        col_height = self.height_map[column_num]
        if col_height == len(self.the_board):
            return 0, False
            #self.the_board.insert(0, self.row_gen(self.width))
        col_index = col_height + 1
        row_index = len(self.the_board) - col_index
        self.the_board[row_index][column_num] = value
        self.height_map[column_num] += 1
        heuristic = self.get_best_heuristic(value)
        return heuristic, True # Return the heuristic.

    def last_col(self):
        return (self.last_col_ins, self.last_val_ins)


    def last_move_row_index(self, column_index):
        """ A function to return the row_index of the last move
        for a column_index
        """
        inserted_height = self.height_map[column_index]
        num_rows = len(self.the_board)
        if inserted_height == 0:
            return num_rows - 1
        return num_rows - inserted_height

    def get_best_heuristic(self, symbol):
        """ For a players <symbol>/piece in the board return the length of the
            longest occurence for symbol.
        Return the longest sequence of <symbol>.
        """
        columns = range(self.width) # [0,1,2,3,4,5,6]
        heur_p1, heur_p2 = 0, 0
        get = lambda c_symbol, row_n, col_n: get_heuristic(self.the_board, c_symbol, row_n, col_n, terminal_test=False)
        # Find the max heuristic of each player post quake.

        for sub_height in range(self.height):
            for col_i in range(self.width):
                height = self.height_map[col_i] - sub_height
                if height <= 0 or height == self.height - 1:
                    continue
                value = self.the_board[height-self.height][col_i]
                row_i = self.height - height
                t_p1_heur = get(self.player_1, row_i, col_i)
                t_p2_heur = get(self.player_2, row_i, col_i)
                if t_p1_heur > heur_p1:
                    heur_p1 = t_p1_heur
                if t_p2_heur > heur_p2:
                    heur_p2 = t_p2_heur
        self.players_heuristics[self.player_1] = heur_p1 # Set Value
        self.players_heuristics[self.player_2] = heur_p2 # Set Value
        if not self.players_heuristics.has_key(symbol):
            return 0
        return self.players_heuristics[symbol]



class GameNode(object):
    """
    Node for each game state. I am thinking we can just store the state in
    lists of lists ( each row represents a row ) and we can easily delete the
    first row by popping. We can also append the new rows by simply
    appending to the state list.
    """

    def __init__(self, parent=None):
        """ Inputs:
        parent: a parent GameNode() object of this GameNode.
        This copies the state of <parent> into self.node_board since the child
        has the configuration of the parent plus one move.
        """
        self.state = []
        self.alpha = 0
        if type(parent) is GameNode:
            self.parent = parent
            self.node_board = GameBoard(game_board=parent.node_board)
        else:
            # If we're the root node, set parent to our own reference.
            self.parent = self
            self.node_board = GameBoard()
            # If no parent.  We're first.

    def terminal_check(self):
        self.node_board.terminal_check()

    def getState(self):
        return self.state

    def getParent():
        return self.parent

def get_heuristic(the_board, symbol, row_index, column_index, terminal_test=False):
    """ A function to find the maximum heuristic for a particular position
    and particular players symbol at a specific location of the board.
    Inputs:
        the_board- A nested list of list object representing the board.
        symbol   - The symbol whose heuristic is to be calculated.
        row_index- The index of the list which represents the row.
        column_index- The index of the column to check
    """
    if the_board[row_index][column_index] != symbol:
        return 0
    heuristics = []
    heuristic_func = [heuristic_horizontal, heuristic_vertical, heuristic_diagonal]
    for function in heuristic_func:
        heuristics.append(function(the_board, symbol, row_index, column_index, terminal_test))
    return max(heuristics)

def heuristic_horizontal(the_board, symbol, row_index, column_index, terminal_test=False):
    """ A Method which only checks the number of symbol to the right of indexes.
    Inputs:
        the_board- A nested list of list object representing the board.
        symbol   - The symbol whose heuristic is to be calculated.
        row_index- The index of the list which represents the row.
        column_index- The index of the column to check
    """
    if symbol != the_board[row_index][column_index]:
        return 0
    left_right_vector = ((0, 1), (0, -1))
    directions = [left_right_vector] # Left up diagonal then right down diagonal.
    return heuristic_generic(the_board, symbol, row_index, column_index, directions, terminal_test)

def heuristic_vertical(the_board, symbol, row_index, column_index, terminal_test=False):
    """ A Method which only checks the number of symbol above the indexes.
    Inputs:
        the_board- A nested list of list object representing the board.
        symbol   - The symbol whose heuristic is to be calculated.
        row_index- The index of the list which represents the row.
        column_index- The index of the column to check
    """
    if symbol != the_board[row_index][column_index]:
        return 0
    # Do Left Direction Vector, and Right direction vector for horizontal.
    up_down_vector = ((-1, 0), (1, 0))
    directions = [up_down_vector] # Left up diagonal then right down diagonal.
    return heuristic_generic(the_board, symbol, row_index, column_index, directions, terminal_test)

def heuristic_diagonal(the_board, symbol, row_index, column_index, terminal_test=False):
    """ A Method which checks the number of symbol to the upper diagonals of indexes.
    Inputs:
        the_board- A nested list of list object representing the board.
        symbol   - The symbol whose heuristic is to be calculated.
        row_index- The index of the list which represents the row.
        column_index- The index of the column to check
    """
    if symbol != the_board[row_index][column_index]:
        return 0
    # Do the up negative slope diagonal first then positive slope. left, right upwards.
    left_diagonal, right_diagonal = ((-1, -1), (1, 1)), ((-1, 1), (1, -1))
    directions = [left_diagonal, right_diagonal] # Left up diagonal then right down diagonal.
    return heuristic_generic(the_board, symbol, row_index, column_index, directions, terminal_test)

def heuristic_generic(the_board, symbol, row_index, column_index, dir_vectors, terminal_test=False):
    """ Inputs:
        dir_vectors: list of directions
            [( (v1, y1), (v2, y2)), ( diagonals ) , (verticals) ]
            (v1, y1): Left, (v2, y2): Right. ((v1, y1), (v2, y2)): Horizontal
        Return:
            The sum of tupled direction vectors ((v1, y1), (v2, y2)) for
            consecutive values equal to symbol.
            Counts the number of reoccuring sequences from the source index
            in the direction of dir_vectors

    """
    if symbol != the_board[row_index][column_index]:
        return 0
    # Boundary booleans.  Don't exceed the limits of the matrix.
    # In the loops you need to execute in the right sequence.
    left_bound = lambda: c_ind >= 0
    upper_bound = lambda: r_ind >= 0
    lower_bound = lambda: r_ind < len(the_board)
    right_bound = lambda: c_ind < len(the_board[r_ind]) # Assumes lower/upper bound.
    this_symbol = lambda: the_board[r_ind][c_ind] == symbol # Assumes bound.

    heuristics = [0 for x in range(len(dir_vectors))]
    end_state = []
    for cnt, diagonal in enumerate(dir_vectors):
        end_ind = []
        for diagonal_vector in diagonal:
            r_ind, c_ind = row_index, column_index
            # Don't change the order of lambda calls.  You may violate left to right.
            while(upper_bound() and lower_bound() and left_bound() and right_bound() and this_symbol()):
                heuristics[cnt] += 1
                row_move, column_move = diagonal_vector
                r_ind += row_move
                c_ind += column_move
            end_ind.append((r_ind, c_ind))
        end_state.append(end_ind)
    # Adjust for directions where we can't extend the move further.
    # O| | | |
    # O|X|X|X|: 3 before, Return: 1
    for cnt, plane_vect in enumerate(end_state):
        is_blocked = [False, False]
        for inner_cnt, end_bound in enumerate(plane_vect):
            r_ind, c_ind = end_bound
            if upper_bound() and lower_bound() and left_bound() and right_bound():
                if the_board[r_ind][c_ind] != 0:
                    is_blocked[inner_cnt] = True
            else:
                is_blocked[inner_cnt] = True
        if is_blocked[0] and is_blocked[1]:
            if not terminal_test and heuristics[cnt] < 5: # We win if it's 5 i.e 4
                heuristics[cnt] = 1 # Account for subtraction on return.
    return max(heuristics) - 1


def print_map(the_board):
    """
    Prints out the map like the Pollet example.
    param: a board state
    """
    value_map = {0:' ', 1:'X', 2:'O'}
    row_str = ''
    iterations = 1
    for row in the_board:
        for el in row:
            row_str += '|{}'.format(value_map[el])
            if iterations % 7 == 0:
                row_str += '|\n'
            iterations += 1
    row_str += bottom_row
    print empty_row
    print row_str

def human_player(node, state, player):
    """ The human player function call.
    """
    prompt_start = 'Please enter a slot from 1 to 7 for your move: '
    print prompt_start,
    invalid = True
    while(invalid):
        try:
            start_slot = int(raw_input(''))
            if start_slot <= 0 or start_slot > 7:  # Burp if out of range.
                raise ValueError('Invalid position')
            invalid = False
        except ValueError:
            print 'Invalid Input.  Enter a slot from 1 to 7: ',
            invalid = True
    return start_slot-1

def computer_player(node, state, player):
    """ The computer player function call.
    """
    #move_weight, move = minimax(node, 0, state, player, -float('inf'), float('inf'))
    move_weight, move = minimax(node, 0, state, player, -float('inf'), float('inf'))
    return move


def relative_heuristic(node, state, player):
    """ Get the heuristic for this relative state of the board.
        player: The AI player
        state: The current turn.
    """
    flip_player = {1:2, 2:1}
    column_move, turn = node.node_board.last_col()
    row_index = node.node_board.last_move_row_index(column_move)
    this_board = node.node_board.the_board

    if player == state:
        hval_pc = node.node_board.get_best_heuristic(state)
        h_val_hu = node.node_board.get_best_heuristic(flip_player[state])
    else:
        hval_pc = node.node_board.get_best_heuristic(flip_player[state])
        h_val_hu = node.node_board.get_best_heuristic(state)
    return_value = hval_pc*10 - h_val_hu*10
    return return_value

def this_min(best_val, value, move, last_move):
    """ Get this min yo
    """
    if value < best_val:
        return value, move
    return best_val, last_move

def this_max(best_val, value, move, last_move):
    """ Get this max yo.
    """
    if value > best_val:
        return value, move
    return best_val, last_move

def child_gen(parent, state):
    """ A generator object for the minimax, minimax functions to generate
    childeren.
    """
    flip_state = (1, 0)
    childeren = []
    for child_move in range(7):
        child = copy.deepcopy(parent)
        value, is_valid = child.node_board.insert(child_move, state)
        if not is_valid:
            continue
        #value = child.node_board.get_best_heuristic(state)
        # Game ending move
        if value >= 4:
            value = 40
        childeren.append((value, child_move, child))
    # Sort on index zero.
    childeren.sort(key=lambda tup: tup[0], reverse=True)
    for child in childeren:
        yield child

def minimax(parent, depth, state, player, alpha, beta):
    QUAKE_STATE = 20
    flip_player = {2:1, 1:2}

    if parent.terminal_check() or depth == 7:
        ret_val = relative_heuristic(parent, state, player)
        return ret_val, None

    max_player = False
    if player == state:
        max_player = True

    best_move = None
    if max_player: # Max Player
        best_value = -float('inf')
        for value, child_move, child in child_gen(parent, state):
            if value < 40: # Terminal Check
                value, t_move = minimax(child, depth + 1, flip_player[state], player, alpha, beta)
            best_value, best_move = this_max(best_value, value, child_move, best_move)
            alpha = best_value
            # ALPHA/BETA CHECK
            if (best_move is not None and beta <= alpha): # IF I'm Max.
                break

        return best_value, best_move
    else: # Min Player
        best_value = float('inf')
        for value, child_move, child in child_gen(parent, state):
            if value >= 40: # Terminal Check
                value = -value
            else:
                value, t_move = minimax(child, depth + 1, flip_player[state], player, alpha, beta)
            best_value, best_move = this_min(best_value, value, child_move, best_move)
            beta = best_value
            # ALPHA/BETA CHECK
            if (best_move is not None and beta <= alpha): # If I'm Min.
                break
        return best_value, best_move



def human_victory_message():
    print '\n\nHow has this happened?!!?  I have be beaten by this creature??'
    print '                 You win this time human.\n'

def computer_victory_message():
    print '\n\nSilly Human.  You will never beat me.  Go get me a sandwich.'
    print '       I have better things to do.  Be gone with you\n'

class ConnectFour(object):

    """ A python class to do something for our assignment.
    """
    fmat_row = '|{}|{}|{}|{}|{}|{}|{}|'
    empty_row = '| | | | | | | |'
    base_row = '|1|2|3|4|5|6|7|'
    # A map to assign the Character symbols to the value.
    symbol_map = {0:'', 1:'X', 2:'O'}
    # A Map to flip the turn. Hopefully Zero will cause an exception.
    opposite_turn = {0:0, 1:2, 2:1}
    first_player, second_player = 1, 2
    root_depth = 0

    def __init__(self, player_first=False, indicate_quake=False):
        """ Inputs:
                player_first: Boolean True condition for the player to go
                    first or False for computer to go first.
              indicate_quake: Boolean True for computer to indicate when
                    a quake happens or False to ask if a quake happened.
        """
        player_1, player_2 = self.first_player, self.second_player
        self.player_first, self.indicate_quake = player_first, indicate_quake
        self.root_state = GameNode()
        self.konnector_brain = GameSocket()
        self.tree_map, self.turn_map, self.quake_map = {}, {}, {}
        self.turn = player_1
        # Use left alignment in a list to keep order of who went where, with what.
        self.previous_move = {player_1:[], player_2:[]}
        if player_first:
            # Dictionaries contain function pointers and player messages.
            self.messages = {player_1: human_victory_message, player_2: computer_victory_message}
            self.player_map = {player_1: human_player, player_2: computer_player}
            self.move_map = {player_1:'Your Move', player_2:'My Move'}
            self.print_map()
            #start_slot = self.player_map[player_1](None, 1, 1) # Key out the function.
        else: # Else it's the computer first.
            # Dictionaries contain function pointers
            self.messages = {player_1: computer_victory_message, player_2: human_victory_message}
            self.player_map = {player_1: computer_player, player_2: human_player}
            self.move_map = {player_1:'My Move', player_2:'Your Move'}
            start_slot = 3 # The middle is the best possible opening move.
        if self.move_map[1] == 'My Move':
            self.insert(start_slot, player_1)
            # Append the first move for player_1
            self.previous_move[player_1].append(start_slot)
        self.print_map()
        #self.state_machine()

    def print_map(self, no_move=False):
        """ A function to print the map in it's current state.
        """
        this_move = self.move_map[self.turn]
        if not no_move: # If a player
            print 'Board After {}:'.format(this_move)
        print_map(self.root_state.node_board.the_board)

    def insert(self, column, player):
        """ A function to insert a players move in the correct column.
            Inputs:
                player: String with 'X' or 'O'
                column: Integer value with domain [0, 7]
        """
        if self.turn != player:
            self.turn = player
        return self.root_state.node_board.insert(column, player)

    def state_machine(self):
        """ A function to handle the turn taking process between player
        and the algorithm.
        """
        board_state = self.root_state.node_board.the_board
        #self.turn = self.opposite_turn[self.turn]
        while(True):
            # Stop Condition for the game or go to next state.
            if self._next_turn_event():
                break
        # END loop of state machine.
        self.messages[self.turn]()


    def _next_turn_event(self):
        """ A function to house the next state code.  Grabs the players
        selected move, inserts into the board, prints the map, and flips
        the turn.
        """
        # get_opponents_move() == self.player_map()
        valid = False
        while(not valid):
            if 'My Move' in self.move_map[self.turn]:
                print 'robot'
                column_move = self.player_map[self.turn](self.root_state, self.turn, self.turn) # Key out the function.
                self.konnector_brain._send_data(6 - column_move)
                #self.konnector_brain._send_data(8-(column_move))
            else:
                column_move = 6 - self.konnector_brain.get_move()
                if column_move is None:
                    # We didn't get a move so end.
                    return True
            end_game, valid = self.insert(column_move, self.turn) # Insert checks the move's heuristic.

        # append this players/turn's move into the ordered list.
        self.previous_move[self.turn].append(column_move)
        self.print_map()
        row_index = self.root_state.node_board.last_move_row_index(column_move)
        this_board = self.root_state.node_board.the_board
        #end_game = self.root_state.node_board.get_best_heuristic(self.turn)
        if end_game >= 4: # A Winner!!
            return True # Stop the state machine!
        self.turn = self.opposite_turn[self.turn]
        return False # Keep going!


if __name__ == '__main__':
    import copy
    ARGS = copy.deepcopy(sys.argv)
    G2 = GameNode()
    G2.node_board.insert(0, 1)
    G2.node_board.insert(0, 1)
    G2.node_board.insert(0, 1)
    G2.node_board.insert(3, 2)
    G2.node_board.insert(6, 1)
    print G2.node_board
