/*~ ~*/

var roadpieces[12][13];
var is_on[12];
var car_x = 6;
var car_y = 2;
var ref_point = 4;
var keep_state = 2;
var loop_counter = 0;
var points = 0;
var loose_state = false;
var distance;

function init() {
	reset();
	set_controls(0b00001010);
}

function game_loop() {

	if (loose_state) return;
	points++;
	if (is_animation_running()) return;
	set_tps(get_current_tps() + 0.013 / (get_current_tps() / 10));
	set_status(str::format("Points: %d", points));
	if (keep_state < 0 && random(0, 10) > 4) {
		if (random(0, 9) > 4) {
			if (ref_point > 0) {
				ref_point--;
				keep_state = 7;
			}
		} else {
			if (ref_point < 6) {
				ref_point++;
				keep_state = 7;
			}
		}
	}
	keep_state--;

	for (var y = 0; y < 12; y++) {
		for (var x = 0; x < 12; x++) {
			roadpieces[x][y] = roadpieces[x][y + 1];
		}
	}

	for (var i = 0; i < 12; i++) {
		roadpieces[i][12] = false;
	}

	roadpieces[ref_point][12] = true;

	if (points > 300) {
		distance = 4;
	} else {
		distance = 5;
	}
	roadpieces[ref_point + distance][12] = true;

	if (loop_counter >= 3) {
		loop_counter = 0;
	}

	for (var i = 0; i < 12; i++) {
		is_on[i] = false;
		if ((i + loop_counter) % 3 == 0) {
			is_on[i] = true;
		}
	}
	loop_counter++;

	if (roadpieces[car_x][car_y] || roadpieces[car_x][car_y + 1]) {
		loose_state = true;
		set_controls(0b00100000);
		run_animation_splash(car_x, car_y, 0xFF0000, true, 1000, 1000);
	}
}

function draw() {
	if (is_animation_running()) return;

	if (loose_state) {
		clear();
		number(1, 4, points, 0xFF0000);
		return;
	}

	for (var y = 0; y < 12; y++) {
		for (var x = 0; x < 12; x++) {
			if (roadpieces[x][y]) {
				if (is_on[y]) {
					set(x, y, 0x00FF00);
				} else {
					set(x, y, 0xFFFFFF);
				}
			} else {
				off(x, y);
			}
		}
	}

	set(car_x, car_y, 0xFFFF00);
	set(car_x, car_y + 1, 0xFFFF00);
}

function clean_up() {
}

function on_event(event) {
	if (event == 2) {
		if (car_x > 0) {
			car_x--;
		}
	}


	if (event == 3) {
		if (car_x < 11) {
			car_x++;
		}

	}

	if (event == 5) {
		loose_state = false;
		reset();
		set_controls(0b00001010);
	}
}

function reset() {
	car_x = 5;
	car_y = 2;
	ref_point = 4;
	keep_state = 2;
	loop_counter = 0;
	points = 0;

	for (var x = 0; x < 12; x++) {
		for (var y = 0; y < 13; y++) {
			roadpieces[x][y] = y == 12 && (x <= ref_point || x >= ref_point + 5);
		}
	}

	for (var i = 0; i < 12; i++) {
		is_on[i] = false;
	}
	set_tps(10);
}