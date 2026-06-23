#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <windows.h>
uint8_t ram[4096];
uint16_t I;
bool keys[16];
void render() {
	if (drawFlag) {
		COORD pos = { 0, 0 };
		SetConsoleCursorPosition(hConsole, pos);

		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 64; x++) {
				std::cout << (screen[y * 64 + x] ? '#' : ' ');
			}
			std::cout << "\n";
		}

		drawFlag = false;
	}
}
void update_keys() {
	keys[0x1] = GetAsyncKeyState('1') & 0x8000;
	keys[0x2] = GetAsyncKeyState('2') & 0x8000;
	keys[0x3] = GetAsyncKeyState('3') & 0x8000;
	keys[0xC] = GetAsyncKeyState('4') & 0x8000; // C = 4
	keys[0x4] = GetAsyncKeyState('Q') & 0x8000;
	keys[0x5] = GetAsyncKeyState('W') & 0x8000;
	keys[0x6] = GetAsyncKeyState('E') & 0x8000;
	keys[0xD] = GetAsyncKeyState('R') & 0x8000; // D = R
	keys[0x7] = GetAsyncKeyState('A') & 0x8000;
	keys[0x8] = GetAsyncKeyState('S') & 0x8000;
	keys[0x9] = GetAsyncKeyState('D') & 0x8000;
	keys[0xE] = GetAsyncKeyState('F') & 0x8000; // E = F
	keys[0xA] = GetAsyncKeyState('Z') & 0x8000; // A = Z
	keys[0x0] = GetAsyncKeyState('X') & 0x8000;
	keys[0xB] = GetAsyncKeyState('C') & 0x8000;
	keys[0xF] = GetAsyncKeyState('V') & 0x8000; // F = V

}
void load_rom(const char* filename) {
	FILE* path = fopen(filename, "rb"); // file name + mode:read binary
	fseek(path, 0, SEEK_END); // to the end
	long siz = ftell(path);
	fseek(path, 0, SEEK_SET); // to the start
	size_t read = fread(&ram[0x200], 1, siz, path);
}
int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr); 
	uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	for (int i = 0; i < 80; i++) {
		ram[i] = fontset[i];
	}
	int dt = 0;
	int st = 0;
	uint8_t V[16] = { 0 };
	uintptr_t pc = 0x200;
	uintptr_t stck[1000];
	uint8_t screen[64 * 32] = { 0 };
	int chk = 0;
	std::string ss;
	std::cin >> ss;
	load_rom(ss.c_str());
	short chks = 0;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	thread th(render);
	while (true) {
		chks += 1;
		update_keys();
		uint16_t opcode = (ram[pc] << 8) | (ram[pc + 1]);

		if (opcode == 0x00E0) {
			COORD pos = { 0, 0 };
			SetConsoleCursorPosition(hConsole, pos);
		}
		else if (opcode == 0x00EE) {
			pc = stck[--chk];
		}
		else {
			uintptr_t c = opcode & 0xF000;
			if (c == 0x1000) {
				pc = opcode & 0x0FFF;
			}
			else if (c == 0x2000) {
				uintptr_t kuda = opcode & 0x0FFF;
				stck[chk] = pc;
				chk = chk += 1;
			}
			else if (c == 0x3000) {
				uint8_t x = (opcode >> 8) & 0x0F;
				uint8_t nn = opcode & 0x00FF;
				if (V[x] == nn) {
					pc = pc + 2;
					pc = kuda;
				}
			}
			else if (c == 0x4000) {
				uint8_t X = (opcode >> 8) & 0x0F;
				uint8_t NN = opcode & 0x00FF;
				if (V[X] != NN) {
					pc += 2;
				}
			}
			else if (c == 0x5000) {
				uint8_t X = (opcode >> 8) & 0x0F;
				uint8_t Y = (opcode >> 4) & 0x0F;
				if (V[X] == V[Y]) {
					pc += 2;
				}
			}
			else if (c == 0x6000) {
				uint8_t zipa = (opcode >> 8) & 0x0F;
				uint8_t wndass = opcode & 0x00FF;
				V[zipa] = wndass;
			}
			else if (c == 0x7000) {
				uint8_t zipa = (opcode >> 8) & 0x0F;
				uint8_t wndass = opcode & 0x00FF;
				V[zipa] = V[zipa] + wndass;
			}
			else if (c == 0xA000) {
				I = opcode & 0x0FFF;
			}
			else if (c == 0xB000) {
				uint16_t b = opcode & 0x0FFF;
				pc = b + V[0];
			}
			else if (c == 0xC000) {
				uint8_t x11 = (opcode >> 8) & 0x0F;
				uint8_t kk = opcode & 0x00FF;
				V[x11] = (rand() % 256) & kk;
			}
			else if (c == 0xD000) {
				uint8_t X = (opcode >> 8) & 0x0F;
				uint8_t Y = (opcode >> 4) & 0x0F;
				uint8_t n = opcode & 0x000F;
				uint8_t xPos = V[X] % 64;
				uint8_t yPos = V[Y] % 32;
				V[0xF] = 0;
				for (int row = 0; row < n; row++) {
					uint8_t spriteByte = ram[I + row];
					for (int col = 0; col < 8; col++) {
						if (spriteByte & (0x80 >> col)) {
							int idx = (yPos + row) * 64 + (xPos + col);
							if (idx < 64 * 32) {
								if (screen[idx] == 1) {
									V[0xF] = 1;
								}
								screen[idx] ^= 1;
							}
						}
					}
				}
				COORD pos = { 0, 0 };
				SetConsoleCursorPosition(hConsole, pos);
				for (int y = 0; y < 32; y++) {
					for (int x = 0; x < 64; x++) {
						std::cout << (screen[y * 64 + x] ? '#' : ' ');
					}
					std::cout << std::endl;
				}
			}
			else {
				uintptr_t c2 = opcode & 0xF00F;
				if (c2 == 0x8000) {
					uint8_t wnd = (opcode >> 8) & 0x0F;
					uint8_t shopa = (opcode >> 4) & 0x0F;
					V[wnd] = V[shopa];
				}
				else if (c2 == 0x8001) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					uint8_t ress = V[X1] | V[Y1];
					V[X1] = ress;
				}
				else if (c2 == 0x8002) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					uint8_t ress = V[X1] & V[Y1];
					V[X1] = ress;
				}
				else if (c2 == 0x8003) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					uint8_t ress = V[X1] ^ V[Y1];
					V[X1] = ress;
				}
				else if (c2 == 0x8004) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					uint16_t ress = V[X1] + V[Y1];
					if (ress > 255) { V[0xF] = 1; }
					else { V[0xF] = 0; }
					V[X1] = ress & 0xFF;
				}
				else if (c2 == 0x8005) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					V[X1] = V[X1] - V[Y1];
					if (V[X1] > V[Y1]) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					}
				}
				else if (c2 == 0x8006) {
					uint8_t X = (opcode >> 8) & 0x0F;
					V[0xF] = V[X] & 0x1;
					V[X] = V[X] >> 1;
					uint8_t Y = (opcode >> 4) & 0x0F;
				}
				else if (c2 == 0x8007) {
					uint8_t X1 = (opcode >> 8) & 0x0F;
					uint8_t Y1 = (opcode >> 4) & 0x0F;
					if (V[Y1] > V[X1]) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					}
					V[X1] = V[Y1] - V[X1];
				}
				else if (c2 == 0x800E) {
					uint8_t X = (opcode >> 8) & 0x0F;
					if (V[X] & 0x80) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					}
					V[X] = V[X] << 1;
				}
				else if (c2 == 0x9001) {
					uint8_t x = (opcode >> 8) & 0x0F;
					uint8_t y = (opcode >> 4) & 0x0F;
					if (V[x] != V[y]) {
						pc += 2;
					}
				}
				else {
					uint8_t c3 = opcode & 0xF0FF;
					if (c3 == 0xF015) {
						uint8_t x = (opcode >> 8) & 0x0F;
						V[x] = dt;
					}
					else if (c == 0xE09E) {
						uint8_t x = (opcode >> 8) & 0x0F;
						if (keys[V[x]]) {
							pc += 2;
						}


					}
					else if ( (opcode & 0xF0FF) == 0xE0A1) {
						uint8_t x = (opcode >> 8) & 0x0F;
						if (!keys[V[x]]) {
							pc += 2;
						}
					}
					else if (c == 0xF007) {
						uint8_t x = (opcode >> 8) & 0x0F;
						V[x] = dt;

					}
					else if (c == 0xF015) {
						uint8_t x = (opcode >> 8) & 0x0F;
						st = V[x];
					}
					else if (c == 0xF01E) {
						x = (opcode >> 8) & 0x0F;
						I = I + V[x];

					}
					else if (c == 0xF029) {
						uint8_t X = (opcode >> 8) & 0x0F;
						I = V[X] * 5; // шрифт начинается с адреса 0x000
					}
					else if (c == 0xF033) {
						x = (opcode >> 8) & 0x0F;
						uint8_t res = V[x];
						ram[I] = res / 100;
						ram[I + 1] = (res / 10) % 10;
						ram[I + 2] = res % 10;

					}
					else if (c == 0xF055) {
						uint8_t x = (opcode >> 8) & 0x0F; //stole the value of x
						for (int i = 0; i <= x; i++) {
							ram[I + i] = V[i];
						}
					}
					else if (c == 0xF065) {
						uint8_t x = (opcode >> 8);
						for (int i = 0; i <= x; i++) {
							V[i] = ram[I + i];

						}
					}
					else if (c == 0xF00A) {
						uint8_t X = (opcode >> 8) & 0x0F;
						bool pressed = false;
						std::cout << "PRESS KEY:" << V[X] << endl;
						for (int i = 0; i < 16; i++) {
							if (keys[i]) {
								V[X] = i;
								pressed = true;
								break;
							}
						}
						if (!pressed) {
							pc -= 2; // повторяем эту же инструкцию, пока не нажмут
						}
					}
				}
			}
		}
		if (chks == 60) {
			if (dt > 0) dt--;
			if (st > 0) st--;
			chks = 0;
		}
		Sleep(1);
	}
}
