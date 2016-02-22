////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//
#include <stdio.h>
#include <time.h>

namespace octet {
	class sprite {
		// where is our sprite (overkill for a 2D game!)
		mat4t modelToWorld;

		// half the width of the sprite
		float halfWidth;

		// half the height of the sprite
		float halfHeight;

		// what texture is on our sprite
		int texture;

		// true if this sprite is enabled.
		bool enabled;
	public:
		sprite() {
			texture = 0;
			enabled = true;
		}

		void init(int _texture, float x, float y, float w, float h) {
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			halfWidth = w * 0.5f;
			halfHeight = h * 0.5f;
			texture = _texture;
			enabled = true;
		}

		void render(intro_to_prog1_shader &shader, mat4t &cameraToWorld) {
			// invisible sprite... used for gameplay.
			if (!texture) return;

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// use "old skool" rendering
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			shader.render(modelToProjection, 0);

			// this is an array of the positions of the corners of the sprite in 3D
			// a straight "float" here means this array is being generated here at runtime.
			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth,  halfHeight, 0,
				-halfWidth,  halfHeight, 0,
			};

			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			// this is an array of the positions of the corners of the texture in 2D
			static const float uvs[] = {
				0,  0,
				1,  0,
				1,  1,
				0,  1,
			};

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			// finally, draw the sprite (4 vertices)
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}

		// move the object
		void translate(float x, float y) {
			modelToWorld.translate(x, y, 0);
		}

		// position the object relative to another.
		void set_relative(sprite &rhs, float x, float y) {
			modelToWorld = rhs.modelToWorld;
			modelToWorld.translate(x, y, 0);
		}

		// return true if this sprite collides with another.
		// note the "const"s which say we do not modify either sprite
		bool collides_with(const sprite &rhs) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
			float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

			// both distances have to be under the sum of the halfwidths
			// for a collision
			return
				(fabsf(dx) < halfWidth + rhs.halfWidth) &&
				(fabsf(dy) < halfHeight + rhs.halfHeight)
				;
		}

		bool is_above(const sprite &rhs, float margin) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

			return
				(fabsf(dx) < halfWidth + margin)
				;
		}

		bool &is_enabled() {
			return enabled;
		}
	};


	class bullet_stor {
	public:
		int angle = 0;
		float lastX = 0.0f;
		float lastY = 0.0f;

		float addAngle(float ang) {
			angle += ang;
			if (angle >= 360) angle = 0;
			if (angle <= -1) angle = 359;
			return angle;
		}
	};

	class bullet_pattern {

		float facing;
		float angle_mul;
		float quad_mul;
		int curve_type;

	public:

		bullet_pattern() {}

		bullet_pattern(int ctype, float face, float amul, float qmul) {
			facing = face; // not sure aqbout this yet..
			angle_mul = amul;
			curve_type = ctype;
			quad_mul = qmul;
		}

		float getXPos(float time) {
			if (curve_type == 0) {
				return angle_mul * (1 / (std::cosf(time * quad_mul)));
			}
			else if (curve_type == 1) {
				return angle_mul * (1 / (std::sinf(time * quad_mul)));
			}
			else if (curve_type == 2) {
				return angle_mul * (1 / (std::tanf(time * quad_mul)));
			}
		}

		float getYPos(float time) {
			if (curve_type == 0) {
				return angle_mul * std::cosf(time * quad_mul);
			}
			else if (curve_type == 1) {
				return angle_mul * std::sinf(time * quad_mul);
			}
			else if (curve_type == 2) {
				return angle_mul * std::tanf(time * quad_mul);
			}
		}
	};


	class csv_reader {

	public:
		std::vector<int> inv_type;
		std::vector<int> inv_spd;
		std::vector<int> inv_pos;
		std::vector<int> inv_hp;
		std::vector<int> inv_spwn;

		csv_reader() {
			init();
		}

	private:
		void init() {
			std::ifstream is("test.csv");

			if (is.bad()) {
				printf("Error: CSV Error");
				return;
			}

			// store the line here
			char buffer[2048];

			// loop over lines
			while (!is.eof()) {
				is.getline(buffer, sizeof(buffer));

				// loop over columns
				char *b = buffer;
				for (int col = 0; ; ++col) {
					char *e = b;
					while (*e != 0 && *e != ',') ++e;
					std::string temp = string(b, e - b);
					//printf("%i = column \n ", col);
					// now b -> e contains the chars in a column
					if (col == 0) {
						inv_type.push_back(std::atoi(temp.c_str()));
					}
					else if (col == 1) {
						inv_hp.push_back(std::atoi(temp.c_str()));
					}
					else if (col == 2) {
						inv_pos.push_back(std::atoi(temp.c_str()));
					}
					else if (col == 3) {
						inv_spd.push_back(std::atoi(temp.c_str()));
					}
					else if (col == 4) {
						inv_spwn.push_back(std::atoi(temp.c_str()));
					}
					if (*e != ',') break;
					b = e + 1;
				}
			}
			return;
		}

	};


	class intro_to_prog_1 : public octet::app {
		// Matrix to transform points in our camera space to the world.
		// This lets us move our camera
		mat4t cameraToWorld;

		// shader to draw a textured triangle
		intro_to_prog1_shader texture_shader_;

		enum {
			num_sound_sources = 8,
			num_rows = 5,
			num_cols = 10,
			num_missiles = 30,
			num_bombs = 30,
			num_borders = 4,
			num_invaderers = 10,

			// sprite definitions
			ship_sprite = 0,
			game_over_sprite,

			first_invaderer_sprite,
			last_invaderer_sprite = first_invaderer_sprite + num_invaderers - 1,

			first_missile_sprite,
			last_missile_sprite = first_missile_sprite + num_missiles - 1,

			first_bomb_sprite,
			last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

			first_border_sprite,
			last_border_sprite = first_border_sprite + num_borders - 1,

			num_sprites,

		};

		//Bullet Pattern Vars
		//Personal note: misile = from ship, bomb = from invader
		bullet_pattern bpShipA;
		bullet_pattern bpShipB;
		bullet_pattern bpInvA;
		bullet_pattern bpInvB;
		bullet_stor bombs[num_bombs];
		bullet_stor missiles[num_missiles];

		//To be used with timer
		double secondsFrom;
		int lastDeltaTime;
		int spwnInvaders;

		//csv reader obj
		csv_reader csvr;

		// timers for missiles and bombs
		int missiles_disabled;
		int bombs_disabled;

		// accounting for bad guys
		int live_invaderers;
		int num_lives;

		// game state
		bool game_over;
		int score;

		// speed of enemy
		float invader_velocity;
		float invader_yvel;
		float ship_speed = 0.025f;

		// sounds
		ALuint whoosh;
		ALuint bang;
		unsigned cur_source;
		ALuint sources[num_sound_sources];

		// big array of sprites
		sprite sprites[num_sprites];

		// random number generator
		class random randomizer;

		// a texture for our text
		GLuint font_texture;

		// information for our text
		bitmap_font font;

		ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

		// called when we hit an enemy
		void on_hit_invaderer() {
			live_invaderers--;
			score++;
			if (live_invaderers == 5) {
				ship_speed *= 2;
			}
			else if (live_invaderers == 0) {
				game_over = true;
				sprites[game_over_sprite].translate(-20, 0);
			}
		}

		// called when we are hit
		void on_hit_ship() {
			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, bang);
			alSourcePlay(source);

			if (--num_lives == 0) {
				game_over = true;
				sprites[game_over_sprite].translate(-20, 0);
			}
		}

		// use the keyboard to move the ship
		void move_ship() {
			// left and right arrows
			if (is_key_down(key_left) || is_key_down(key_a)) {
				sprites[ship_sprite].translate(-ship_speed, 0);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 2])) {
					sprites[ship_sprite].translate(+ship_speed, 0);
				}
			}
			else if (is_key_down(key_right) || is_key_down(key_d)) {
				sprites[ship_sprite].translate(+ship_speed, 0);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 3])) {
					sprites[ship_sprite].translate(-ship_speed, 0);
				}
			}
			if (is_key_down(key_up) || is_key_down(key_w)) {
				sprites[ship_sprite].translate(0, +ship_speed);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 1])) {
					sprites[ship_sprite].translate(0, -ship_speed);
				}
			}
			else if (is_key_down(key_down) || is_key_down(key_s)) {
				sprites[ship_sprite].translate(0, -ship_speed);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 0])) {
					sprites[ship_sprite].translate(0, +ship_speed);
				}
			}
		}

		// fire button (space)
		void fire_missiles() {
			if (missiles_disabled) {
				--missiles_disabled;
			}
			else if (is_key_down(' ')) {
				// find a missile
				for (int i = 0; i != num_missiles; ++i) {
					if (!sprites[first_missile_sprite + i].is_enabled()) {
						sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0.5f * (math::sin((getTime() - secondsFrom) * 180)), 0.5f);

						//sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0, 0.5f);			
						sprites[first_missile_sprite + i].is_enabled() = true;
						if (live_invaderers <= 5) {
							missiles_disabled = 5;
						}
						else {
							missiles_disabled = 8;
						}
						ALuint source = get_sound_source();
						alSourcei(source, AL_BUFFER, whoosh);
						alSourcePlay(source);
						break;
					}
				}
			}
		}

		// pick and invader and fire a bomb
		void fire_bombs() {
			if (bombs_disabled) {
				--bombs_disabled;
			}
			else {
				// find an invaderer
				sprite &ship = sprites[ship_sprite];
				for (int j = randomizer.get(0, num_invaderers); j < num_invaderers; ++j) {
					sprite &invaderer = sprites[first_invaderer_sprite + j];
					if (invaderer.is_enabled() /*&& invaderer.is_above(ship, 0.3f)*/) {
						// find a bomb
						for (int i = 0; i != num_bombs; ++i) {
							if (!sprites[first_bomb_sprite + i].is_enabled()) {
								sprites[first_bomb_sprite + i].set_relative(invaderer, 0.5f * (math::sin((getTime() - secondsFrom) * 180)), -0.25f);
								sprites[first_bomb_sprite + i].is_enabled() = true;
								bombs_disabled = 10;
								ALuint source = get_sound_source();
								alSourcei(source, AL_BUFFER, whoosh);
								alSourcePlay(source);
								return;
							}
						}
						return;
					}
				}
			}
		}

		// animate the missiles
		void move_missiles() {
			const float missile_speed = 0.1f;
			for (int i = 0; i != num_missiles; ++i) {
				sprite &missile = sprites[first_missile_sprite + i];
				if (missile.is_enabled()) {
					/*
					if (live_invaderers <= 5) {
					missiles[i].addAngle(5);
					missile.translate(bpShipB.getXPos(missiles[i].angle) - missiles[i].lastX, bpShipA.getYPos(missiles[i].angle) - missiles[i].lastY);
					}
					else {
					missiles[i].addAngle(5);
					missile.translate(bpShipA.getXPos(missiles[i].angle) - missiles[i].lastX, bpShipA.getYPos(missiles[i].angle) - missiles[i].lastY);
					}*/
					missile.translate(0, missile_speed);
					for (int j = 0; j != num_invaderers; ++j) {
						sprite &invaderer = sprites[first_invaderer_sprite + j];
						if (invaderer.is_enabled() && missile.collides_with(invaderer)) {
							csvr.inv_hp[j] -= 10;
							if (csvr.inv_hp[j] < 1) {
								invaderer.is_enabled() = false;
								invaderer.translate(20, 0);
								ALuint source = get_sound_source();
								alSourcei(source, AL_BUFFER, bang);
								alSourcePlay(source);
								on_hit_invaderer();
							}
							missile.is_enabled() = false;
							missile.translate(20, 0);
							goto next_missile;
						}
					}
					if (missile.collides_with(sprites[first_border_sprite + 1])) {
						missile.is_enabled() = false;
						missile.translate(20, 0);
					}
				}
			next_missile:;
			}
		}

		// animate the bombs
		void move_bombs() {
			const float bomb_speed = 0.2f;
			for (int i = 0; i != num_bombs; ++i) {
				sprite &bomb = sprites[first_bomb_sprite + i];
				if (bomb.is_enabled()) {
					bomb.translate(0, -bomb_speed);
					if (bomb.collides_with(sprites[ship_sprite])) {
						bomb.is_enabled() = false;
						bomb.translate(20, 0);
						bombs_disabled = 50;
						on_hit_ship();
						goto next_bomb;
					}
					if (bomb.collides_with(sprites[first_border_sprite + 0])) {
						bomb.is_enabled() = false;
						bomb.translate(20, 0);
					}
				}
			next_bomb:;
			}
		}

		// move the array of enemies
		void move_invaders(float dx, float dy) {
			for (int j = 0; j != num_invaderers; ++j) {
				sprite &invaderer = sprites[first_invaderer_sprite + j];
				if (invaderer.is_enabled()) {
					if (csvr.inv_type[j] == 0) {
						invaderer.translate(dx * csvr.inv_spd[j], dy);
					}
					else {
						invaderer.translate(dx * csvr.inv_spd[j], 0);
					}
				}
			}
		}

		// check if any invaders hit the sides.
		bool invaders_collide(sprite &border) {
			for (int j = 0; j != num_invaderers; ++j) {
				sprite &invaderer = sprites[first_invaderer_sprite + j];
				if (invaderer.is_enabled() && invaderer.collides_with(sprites[first_border_sprite + 0])) {
					invaderer.is_enabled() = false;
					invaderer.translate(20, 0);
					live_invaderers--;
				}
				if (invaderer.is_enabled() && invaderer.collides_with(border)) {
					return true;
				}
			}
			return false;
		}


		void draw_text(intro_to_prog1_shader &shader, float x, float y, float scale, const char *text) {
			mat4t modelToWorld;
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			modelToWorld.scale(scale, scale, 1);
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			/*mat4t tmp;
			glLoadIdentity();
			glTranslatef(x, y, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
			glScalef(scale, scale, 1);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

			enum { max_quads = 32 };
			bitmap_font::vertex vertices[max_quads * 4];
			uint32_t indices[max_quads * 6];
			aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

			unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, font_texture);

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
			glEnableVertexAttribArray(attribute_pos);
			glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
			glEnableVertexAttribArray(attribute_uv);

			glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
		}

	public:

		// this is called when we construct the class
		intro_to_prog_1(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
		}

		// this is called once OpenGL is initialized
		void app_init() {

			//set up timer init
			init_spawner();

			// set up the shader
			texture_shader_.init();

			// set up the matrices with a camera 5 units from the origin
			cameraToWorld.loadIdentity();
			cameraToWorld.translate(0, 0, 3);

			font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

			GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
			ship_speed = 0.025f;
			sprites[ship_sprite].init(ship, 0, -2.75f, 0.25f, 0.25f);

			GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
			sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f);

			GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");
			spwnInvaders = 0;
			invader_yvel = -0.02f;
			for (int i = 0; i != num_invaderers; ++i) {
				assert(first_invaderer_sprite + i <= last_invaderer_sprite);
				if (csvr.inv_type[i] == 0) {
					sprites[first_invaderer_sprite + i].init(invaderer, 20.0f, 0.0f, 0.3f, 0.3f);
				}
				else {
					sprites[first_invaderer_sprite + i].init(invaderer, 20.0f, 0.0f, 0.35f, 0.35f);
				}
				sprites[first_invaderer_sprite + i].is_enabled() = false;
				//sprites[first_invaderer_sprite + i].translate(20, 0);
			}
			//for (int j = 0; j != num_rows; ++j) {
			//  for (int i = 0; i != num_cols; ++i) {
			//    assert(first_invaderer_sprite + i + j*num_cols <= last_invaderer_sprite);
			//    sprites[first_invaderer_sprite + i + j*num_cols].init(
			//      invaderer, ((float)i - num_cols * 0.5f) * 0.5f, 2.50f - ((float)j * 0.5f), 0.25f, 0.25f
			//    );
			//  }
			//}

			// set the border to white for clarity
			GLuint white = resource_dict::get_texture_handle(GL_RGB, "#333333");
			sprites[first_border_sprite + 0].init(white, 0.0f, -3.0f, 6.0f, 0.2f);
			sprites[first_border_sprite + 1].init(white, 0.0f, 3.0f, 6.0f, 0.2f);
			sprites[first_border_sprite + 2].init(white, -3.0f, 0.0f, 0.2f, 6.0f);
			sprites[first_border_sprite + 3].init(white, 3.0f, 0.0f, 0.2f, 6.0f);

			//bullet patterns
			bpInvA = bullet_pattern(1, 90, 2, 3);
			bpInvB = bullet_pattern(2, 90, 2, 3);
			bpShipA = bullet_pattern(0, -90, 2, 3);
			bpShipB = bullet_pattern(0, -90, 3, 5);

			// use the missile texture
			GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
			for (int i = 0; i != num_missiles; ++i) {
				// create missiles off-screen
				sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f);
				sprites[first_missile_sprite + i].is_enabled() = false;
			}

			// use the bomb texture
			GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
			for (int i = 0; i != num_bombs; ++i) {
				// create bombs off-screen
				sprites[first_bomb_sprite + i].init(bomb, 20, 0, 0.0625f, 0.25f);
				sprites[first_bomb_sprite + i].is_enabled() = false;
			}

			// sounds
			whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/whoosh.wav");
			bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
			cur_source = 0;
			alGenSources(num_sound_sources, sources);

			// sundry counters and game state.
			missiles_disabled = 5;
			bombs_disabled = 10;
			invader_velocity = 0.01f;
			live_invaderers = num_invaderers;
			num_lives = 5;
			game_over = false;
			score = 0;
		}

		//Get the start time of the app instance to compare with later
		void init_spawner() {

			secondsFrom = getTime();

		}

		//utility function to get time (in seconds from Jan 1970?
		double getTime() {
			time_t s_time;
			s_time = time(NULL);
			return (double)s_time;
		}


		//Check csv for spawn times and re-enable invaders and position them.
		void spawner() {

			int deltaTime = getTime() - secondsFrom;
			if (deltaTime > lastDeltaTime) {
				lastDeltaTime = deltaTime;
				if (spwnInvaders < csvr.inv_spwn.size() && csvr.inv_spwn[spwnInvaders] <= deltaTime) {
					sprites[first_invaderer_sprite + spwnInvaders].is_enabled() = true;
					sprites[first_invaderer_sprite + spwnInvaders].translate(-22.5 + (csvr.inv_pos[spwnInvaders] * 1), 2.5);
					spwnInvaders++;
				}
			}
		}

		// called every frame to move things
		void simulate() {

			if (game_over) {
				return;
			}

			spawner();

			move_ship();

			fire_missiles();

			fire_bombs();

			move_missiles();

			move_bombs();

			move_invaders(invader_velocity, invader_yvel);

			sprite &border = sprites[first_border_sprite + (invader_velocity < 0 ? 2 : 3)];
			if (invaders_collide(border)) {
				invader_velocity = -invader_velocity;
				move_invaders(invader_velocity, 0.0f);
			}
			if (invader_yvel < 0) {
				sprite &bot_border = sprites[first_border_sprite + 0];
				invaders_collide(bot_border);
			}
		}

		// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			simulate();

			// set a viewport - includes whole window area
			glViewport(x, y, w, h);

			// clear the background to black
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
			glDisable(GL_DEPTH_TEST);

			// allow alpha blend (transparency when alpha channel is 0)
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// draw all the sprites
			for (int i = 0; i != num_sprites; ++i) {
				sprites[i].render(texture_shader_, cameraToWorld);
			}

			char score_text[32];
			sprintf(score_text, "score: %d   lives: %d\n", score, num_lives);
			draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, score_text);

			// move the listener with the camera
			vec4 &cpos = cameraToWorld.w();
			alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
		}
	};
}
