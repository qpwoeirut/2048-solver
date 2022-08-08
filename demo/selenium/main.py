import time
from typing import Tuple, List

import players
from selenium import webdriver
from selenium.common.exceptions import StaleElementReferenceException, NoSuchElementException, \
    ElementNotInteractableException
from selenium.webdriver.common.by import By
from selenium.webdriver.remote.webelement import WebElement

MOVES = "awds"  # 0=left, 1=up, 2=right, 3=down
POWERS_OF_TWO = [1 << x for x in range(18)]  # from 2 to 2^17
LOG2 = {val: i for i, val in enumerate(POWERS_OF_TWO)}


def extract_value_from_classes(classes: List[str]) -> int:
    return int([n for n in POWERS_OF_TWO if f"tile-{n}" in classes][0])


def extract_position_from_classes(classes: List[str]) -> Tuple[int, int]:
    pos_str = [c.lstrip("tile-position-") for c in classes if c.startswith("tile-position-")][0]
    col, row = map(int, pos_str.split('-'))
    return 4 - row, 4 - col


# returns the bitwise representation of the board used in the C++ implementation
# probably will only be used at the start (with 2 tiles on board), but should always work
def read_board(tiles: List[WebElement]) -> int:
    board = 0
    for tile in tiles:
        classes = tile.get_attribute("class").split()
        row, col = extract_position_from_classes(classes)

        idx = (row << 2 | col) * 4
        current_val = (board >> idx) & 0xF
        new_val = LOG2[extract_value_from_classes(classes)]

        # when tiles get merged, they still exist for one move after
        # the correct tile always has a higher value, so we can take the max
        board = (board ^ (current_val << idx)) | (max(current_val, new_val) << idx)
    return board


def play_game(player: object):
    browser = webdriver.Firefox()
    browser.get("https://play2048.co/")

    # keystrokes will be sent to top-level container
    body: WebElement = browser.find_element(By.TAG_NAME, "body")
    game_container: WebElement = browser.find_element(By.CLASS_NAME, "game-container")
    tile_container: WebElement = game_container.find_element(By.CLASS_NAME, "tile-container")

    while len(game_container.find_elements(By.CLASS_NAME, "game-over")) == 0:  # check if game is over
        for _ in range(10):
            try:
                board = read_board(tile_container.find_elements(By.CLASS_NAME, "tile"))
                move = player.pick_move(board)

                old_board = board

                body.send_keys(MOVES[move])
                board = read_board(tile_container.find_elements(By.CLASS_NAME, "tile"))
                if board != old_board:
                    break

                # make sure the board is focused
                game_container.click()
                print("Board did not change!", board, move)
            except StaleElementReferenceException:
                time.sleep(0.025)
        else:
            try:  # maybe we won! click keep going if that's the case
                keep_going_button = game_container.find_element(By.CLASS_NAME, "keep-playing-button")
                keep_going_button.click()
            except (NoSuchElementException, ElementNotInteractableException):
                # let's just print something and then keep going as if nothing happened
                print("PANIK")


def main():
    play_game(players.RandomPlayer())
    # play_game(players.MonteCarloPlayer(5000))

    CORNER_HEURISTIC = 2
    # play_game(players.ExpectimaxDepthStrategy(0, CORNER_HEURISTIC))


if __name__ == '__main__':
    main()
