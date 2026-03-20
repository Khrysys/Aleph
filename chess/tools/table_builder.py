from pathlib import Path
from sys import argv

def rank_file(sq: int) -> tuple[int, int]:
    '''
    Docstring for row_file
    
    :param sq: Integer square index on the board, 0-63. 
    :type sq: int
    :return: A tuple containing the rank and file of the square on a chessboard as (rank, file).
    :rtype: tuple[int, int]
    '''
    return divmod(sq, 8)

def index_for(r: int, f: int) -> int:
    return (r * 8) + f

def flip_square(sq: int) -> int:
    '''
    Docstring for flip_square
    
    :param sq: Integer square index on the board, 0-63.
    :type sq: int
    :return: The index of the square flipped across the horizontal center.
    :rtype: int
    '''
    r, f = rank_file(sq)
    return index_for(7 - r, f)

def flip_bb(bb: int) -> int:
    nbb = 0
    for i in range(64):
        j = flip_square(i)
        nbb |= (bb >> j & 1) << i
    return nbb


def bit_for(sq: int) -> int:
    return 1 << sq

def is_at(bb: int, sq: int) -> bool:
    '''
    Docstring for is_at
    
    :param bb: bitboard of squares, 0-(2^64-1)
    :type bb: int
    :param sq: Integer square index on the board, 0-63.
    :type sq: int
    :return: if the given Square sq is contained in Bitboard bb
    :rtype: bool
    '''
    return ((1 << sq) & bb) != 0

def bit_if_on_board(r: int, f: int) -> int:
    if 0 <= r < 8 and 0 <= f < 8:
        return bit_for(index_for(r, f))
    return 0

class AttackTables:
    between: list[list[int]]
    attacks: list[list[int]]

    def __init__(self):
        self.attacks = self._calculate_attack_tables()
        self.between = self._calculate_between_tables()

    def _calculate_attack_tables(self) -> list[list[int]]:
        wp = self._calculate_pawn_movement()
        n = self._calculate_knight_movement()
        b = self._calculate_bishop_movement()
        r = self._calculate_rook_movement()
        q = self._calculate_queen_movement()
        k = self._calculate_king_movement()

        bp = []
        for i in range(64):
            wsq = flip_square(i)
            bp.append(flip_bb(wp[wsq]))

        # black pawns have to be inverted both on the internal map and on the index. 
        return [wp, n, b, r, q, k, 
                bp, n, b, r, q, k]

    def _calculate_pawn_movement(self) -> list[int]:
        tables = []
        for i in range(64):
            r, f = rank_file(i)
            bb = 0
            bb |= bit_if_on_board(r + 1, f - 1)
            bb |= bit_if_on_board(r + 1, f + 1)
            tables.append(bb)
        return tables
    
    def _calculate_knight_movement(self) -> list[int]:
        tables = []
        DIRECTIONS = [(-2, -1), (-2, 1), (-1, -2), (-1, 2), (1, -2), (1, 2), (2, -1), (2, 1)]

        for i in range(64):
            bb = 0
            r, f = rank_file(i)
            for dr, df in DIRECTIONS:
                bb |= bit_if_on_board(r+dr, f+df)
            tables.append(bb)
        return tables
    
    def _calculate_bishop_movement(self) -> list[int]:
        tables = []
        DIRECTIONS = [(1, 1), (1, -1), (-1, 1), (-1, -1)]
        for i in range(64):
            bb = 0
            sr, sf = rank_file(i)
            for dr, df in DIRECTIONS:
                r = sr + dr
                f = sf + df
                while bit_if_on_board(r, f) != 0:
                    bb |= bit_if_on_board(r, f)
                    r += dr
                    f += df
            tables.append(bb)
        return tables
    
    def _calculate_rook_movement(self) -> list[int]:
        tables = []
        DIRECTIONS = [(0, 1), (0, -1), (1, 0), (-1, 0)]
        for i in range(64):
            bb = 0
            sr, sf = rank_file(i)
            for dr, df in DIRECTIONS:
                r = sr + dr
                f = sf + df
                while bit_if_on_board(r, f) != 0:
                    bb |= bit_if_on_board(r, f)
                    r += dr
                    f += df
            tables.append(bb)
        return tables
    
    def _calculate_queen_movement(self) -> list[int]:
        tables = []
        DIRECTIONS = [(0, 1), (0, -1), (1, 0), (-1, 0), (1, 1), (1, -1), (-1, 1), (-1, -1)]
        for i in range(64):
            bb = 0
            sr, sf = rank_file(i)
            for dr, df in DIRECTIONS:
                r = sr + dr
                f = sf + df
                while bit_if_on_board(r, f) != 0:
                    bb |= bit_if_on_board(r, f)
                    r += dr
                    f += df
            tables.append(bb)
        return tables
    
    def _calculate_king_movement(self) -> list[int]:
        tables = []
        DIRECTIONS = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]

        for i in range(64):
            bb = 0
            r, f = rank_file(i)
            for dr, df in DIRECTIONS:
                bb |= bit_if_on_board(r+dr, f+df)
            tables.append(bb)
        return tables

    def _calculate_between_tables(self) -> list[list[int]]:
        d = []
        d.extend([self._calculate_between_for_square(i) for i in range(64)])
        return d
    
    def _calculate_between_for_square(self, sq: int) -> list[int]:
        d = []
        d.extend([self._calculate_single_between(sq, i)for i in range(64)])
        return d
    
    def _calculate_single_between(self, f: int, t: int) -> int:
        if f == t:
            return 0

        fr, ff = rank_file(f)
        tr, tf = rank_file(t)

        dr = tr - fr
        df = tf - ff

        # Determine direction
        step_r = 0
        step_f = 0

        if dr == 0 and df != 0:
            step_r = 0
            step_f = 1 if df > 0 else -1
        elif df == 0 and dr != 0:
            step_r = 1 if dr > 0 else -1
            step_f = 0
        elif abs(dr) == abs(df):
            step_r = 1 if dr > 0 else -1
            step_f = 1 if df > 0 else -1
        else:
            return 0  # Not aligned

        bb = 0
        r = fr + step_r
        f_ = ff + step_f

        while 0 <= r < 8 and 0 <= f_ < 8:
            sq = index_for(r, f_)
            if sq == t:
                return bb
            bb |= bit_for(sq)
            r += step_r
            f_ += step_f

        return 0

    def _output_header(self) -> str:
        return "/* THIS FILE IS AUTOGENERATED BY chess/tools/table_builder.py\n * DO NOT EDIT THIS FILE BY HAND */\n#pragma once\n\n#include <aleph/chess/attack_tables.hpp>\n\n"
    
    def _output_data(self, table: list[list[int]]) -> str:
        data = "\t\t{{\n"
        for p, tables in enumerate(table):
            data += "\t\t\t{ /* " + str(p + 1) + '*/\n'
            for i, j in enumerate(tables):
                data += ("\t\t\t\t" if i % 8 == 0 else "") + f"0x{j:016X}ULL"
                if i != len(tables) - 1:
                    data += ", "
                if (i + 1) % 8 == 0:
                    data += "\n"
            data += "\t\t\t}"
            if p != len(table) - 1:
                data += ", "
            data += "\n"
        data += "\t\t}}"
        return data

    def _output_blob_data(self) -> str:
        data = "namespace aleph::chess {\n\tconstinit detail::AttackTables attackTables = {\n"
        data += self._output_data(self.attacks) + ', '
        data += self._output_data(self.between)
        data += "\n\t};\n}"
        return data

    def output(self) -> str:
        return self._output_header() + self._output_blob_data()
        

def main():
    file_name = argv[1]
    file_path = Path(file_name)
    file = file_path.open('w')

    file.write(AttackTables().output())

    file.close()



if __name__ == '__main__':
    main()