"""
This module was designed to handle th ecommand line arguments for the
Connect Earthquake assignment implementing a Hill Climbing algorithm.

- Matthew Carlis
"""
import sys

import threading
import time
from four_connect import ConnectFour

CRAP_OUT = '\nERROR with human input.'
CRAP_OUT += '\nYou entered something wrong.  Stop being a Type Noob.'


GAME_OBJ = None

def spawn_thread(player_first=False, indicate_quake=False):
    """
    """
    global GAME_OBJ
    if GAME_OBJ is None:
        GAME_OBJ = ConnectFour(player_first, indicate_quake)
    th = threading.Thread(target=run_simulation)
    th.start()
    usr_in = ''
    try:
        while GAME_OBJ.konnector_brain.alive.isSet():
            usr_in = raw_input("Reset: r/R?")
            if usr_in in ('R', 'r', 'reset', 'Reset'):
                GAME_OBJ.konnector_brain.alive.clear()
                th.join()
                GAME_OBJ = ConnectFour(player_first, indicate_quake)
                th = threading.Thread(target=run_simulation)
                th.start()
                usr_in = ''
    except ValueError:
        print CRAP_OUT
        GAME_OBJ.konnector_brain.alive.clear()
        sys.exit(1)
    except KeyboardInterrupt:
        print '\n\n          I win by your submission human!'
        print 'Maybe you need larger heat sinks to handle this heat\n'
        GAME_OBJ.konnector_brain.alive.clear()
        sys.exit(1)
    except Exception, excep:
        print 'Something Failed:', excep
        GAME_OBJ.konnector_brain.alive.clear()
        sys.exit(1)

def run_simulation():
    """
    """
    if GAME_OBJ is None:
        return
    GAME_OBJ.state_machine()
    GAME_OBJ.konnector_brain.alive.clear()
    return


def parse_arguments(arguments):
    prompt_indicate = 'Should I tell you when earthquake happen (y/n)?\n'
    prompt_indicate += '(If not, each round, I will ask you if an earthquake happened): '
    try:
        player_first = raw_input('Would you like to go first (y/n)?: ').strip(' ') == 'y'
        indicate_quake = False #raw_input(prompt_indicate).strip(' ') == 'y'
    except KeyboardInterrupt:
        sys.exit(1)
    except Exception:
        print CRAP_OUT
        sys.exit(1)

    return (player_first, indicate_quake)


if __name__ == '__main__':
    import copy
    ARGS = copy.deepcopy(sys.argv)
    PLAYER, QUAKE = parse_arguments(ARGS)
    spawn_thread(PLAYER, QUAKE)

