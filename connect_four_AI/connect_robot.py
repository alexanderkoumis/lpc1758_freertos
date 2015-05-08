"""
This module was designed to handle th ecommand line arguments for the
Connect Earthquake assignment implementing a Hill Climbing algorithm.

- Matthew Carlis
"""
import sys

from four_connect import ConnectFour

CRAP_OUT = '\nERROR with human input.'
CRAP_OUT += '\nYou entered something wrong.  Stop being a Type Noob.'

def run_simulation(player_first=False, indicate_quake=False):
    try:
        game_obj = ConnectFour(player_first, indicate_quake)
        game_obj.state_machine()
        game_obj.konnector_brain.alive.clear()
    except ValueError:
        print CRAP_OUT
        sys.exit(1)
    except KeyboardInterrupt:
        print '\n\n          I win by your submission human!'
        print 'Maybe you need larger heat sinks to handle this heat\n'
        game_obj.konnector_brain.alive.clear()
        sys.exit(1)
    except Exception, excep:
        print 'Something Failed:', excep
        game_obj.konnector_brain.alive.clear()
        sys.exit(1)


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
    run_simulation(PLAYER, QUAKE)

