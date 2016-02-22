////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet {
	/// Scene containing a box with octet.
	class math_gfx_1_lsystem : public app {
		// scene for drawing box
		class node {
			vec3 pos;
			float z_angle;
			float y_angle;
			float seg_width;
		public:
			node() {
				pos = vec3(0.0f, 0.0f, 0.0f);
				z_angle = 0.0f;
				y_angle = 0.0f;
				seg_width = 0.0f;
			}

			node(vec3 p, float za, float sw, float ya = 0.0f) {
				pos = p;
				z_angle = za;
				y_angle = ya;
				seg_width = sw;
			}

			vec3& get_pos() {
				return pos;
			}

			float& get_z_angle() {
				return z_angle;
			}

			float& get_y_angle() {
				return y_angle;
			}

			float& get_seg_width() {
				return seg_width;
			}
		};


		//Local Vars
		const float math_PI = 3.14159265f;
		const float min_angle_z = 20.0f;
		const float max_angle_z = 45.0f;
		const int min_step = 0;
		const int max_step = 6;
		const int min_file = 0;
		const int max_file = 8;

		int selected_file_idx = 0;
		int curr_file = 0;
		int curr_step = 0;
		int curr_lvl = 0;
		float z_angle_step = 25.0f;
		float y_angle_step = 20.0f;
		float part_length = 5.0f;
		float part_width = 2.6f;
		bool isSingleMesh = false;
		bool is3D = false;
		float total_y = 0.0f;

		//Stuff to make the scene
		ref<visual_scene> app_scene;
		mouse_look mouse_controller;
		ref<camera_instance> camera;
		dynarray<node> node_list;
		node root_node;

		//Texture materials
		material *red_mat;
		material *brown_mat;
		material *green_mat;
		material *paint_mat;

		//rule storage
		dynarray<char> vars;
		dynarray<char> consts;
		dynarray<char> inits;
		dynarray<char> axioms;
		hash_map<char, dynarray<char>> rules;


  public:
    /// this is called when we construct the class before everything is initialised.
    math_gfx_1_lsystem(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
		//Load Initial LSystem
		curr_step = 0;
		open_file(selected_file_idx);

		//Make Scene
		app_scene = new visual_scene();
		app_scene->create_default_camera_and_lights();
		camera = app_scene->get_camera_instance(0);

		//printf("\n Cam Pos = %f , %f , %f \n", camera->get_node()->get_position().x(), camera->get_node()->get_position().y(), camera->get_node()->get_position().z());
		camera->set_far_plane(500.0f);
		
		//Set Control Mouse
		mouse_controller.init(this, 100.0f / 360.0f, false);

		//color materials139,69,19
		brown_mat = new material(vec4(0.7f, 0.35f, 0.1f, 1.0f));
		green_mat = new material(vec4(0.3f, 0.5f, 0.1f, 1.0f));
		red_mat = new material(vec4(0.8f, 0.2f, 0.1f, 1.0f));
		paint_mat = brown_mat;

		gen_mesh();

    }

	//access file
	void open_file(int file_idx) {
		dynarray<uint8_t> file;
		std::string name = "assets/lsystems/lsystem" + std::to_string(file_idx) + ".txt";
		app_utils::get_url(file, name.c_str());

		get_lsystem(file);
	}

	// get rules from text file into vars
	void get_lsystem(dynarray<uint8_t> file) {

		vars.reset();
		consts.reset();
		inits.reset();
		axioms.reset();
		rules.clear();


		dynarray<uint8_t> new_line;
		for each(uint8_t c in file) {
			if (c != ' ' && c != '\n' && c != '\r') {
				new_line.push_back(c);
			}
		}

		int i = 0;
		// get variables
		for (; i < new_line.size(); ++i) {
			char current_char = new_line[i];
			if (current_char == ';') {
				break;
			}
			else if (current_char == ',') {
				continue;
			}
			else {
				vars.push_back(current_char);
			}
		}
		++i;

		// get constants
		for (; i < new_line.size(); ++i) {
			char char_idx = new_line[i];
			if (char_idx == ';') {
				break;
			}
			else if (char_idx == ',') {
				continue;
			}
			else {
				consts.push_back(char_idx);
			}
		}
		++i;

		// get axiom
		for (; i < new_line.size(); ++i) {
			char char_idx = new_line[i];
			if (char_idx == ';') {
				break;
			}
			else {
				inits.push_back(char_idx);
				axioms.push_back(char_idx);
			}
		}
		++i;

		// get rules
		while (i < new_line.size()) {
			char key = new_line[i];
			i += 3;

			for (; i < new_line.size(); ++i) {
				char char_idx = new_line[i];
				if (char_idx == ';') {
					break;
				}
				else {
					rules[key].push_back(char_idx);
				}
			}
			++i;
		}
	}

	//Tried to use math class to draw single mesh here, but ended up using cylinders. Will possibly try again later.
	void gen_mesh() {
		/*
		
		*/

		part_length = 5.0f;
		part_width = 2.6f;
		//Why do i need to destroy the scene and start over rather than destroy the meshes. Below didnt work.
		//Figure this out later
		//printf("\n %i = Number of Meshes \n", app_scene->get_num_mesh_instances());
		//if (app_scene->get_num_mesh_instances() > 0) {
		//	app_scene->reset_meshes_anims();
		//	dynarray<scene_node*> nodes;
		//	dynarray<int> parents;
		//	app_scene->get_all_child_nodes(nodes, parents);
		//	for (int i = 0; i != nodes.size(); ++i) {
		//		nodes[i]->release();
		//	}
		//}
		vec3 cam_pos;
		if (camera)	cam_pos = camera->get_node()->get_position();
		app_scene = new visual_scene();
		app_scene->create_default_camera_and_lights();
		camera = app_scene->get_camera_instance(0);
		camera->set_far_plane(500.0f);
		printf("\n Cam Pos = %f, %f, %f \n", cam_pos.x(), cam_pos.y(), cam_pos.z());
		camera->get_node()->translate(vec3(cam_pos.x() - camera->get_node()->get_position().x(), cam_pos.y() - camera->get_node()->get_position().y(), cam_pos.z() - camera->get_node()->get_position().z()));
		

		vec3 pos = vec3(0.0f, 0.0f, 0.0f);
		float z_angle = 0.0f;
		float y_angle = 0.0f;
		curr_lvl = 0;

		dynarray<char> axiom = axioms;

		int max_steps = 8;
		int num_parts = total_parts();
		int num_parts_step = num_parts / max_steps;
		int segment_count = 0;

		printf("\n Axiom = ");

		for (unsigned int i = 0; i < axiom.size(); ++i) {
			printf("%c", axiom[i]);
			if (axiom[i] == '+') {
				z_angle += z_angle_step;
				y_angle += y_angle_step;
			}
			else if (axiom[i] == '-') {
				z_angle -= z_angle_step;
				y_angle -= y_angle_step;
			}
			else if (axiom[i] == '[') {
				++curr_lvl;
				node n = node(pos, z_angle, part_width, y_angle);
				node_list.push_back(n);
			}
			else if (axiom[i] == ']') {
				node n = node_list[node_list.size() - 1];
				--curr_lvl;
				node_list.pop_back();
				z_angle = n.get_z_angle();
				y_angle = n.get_y_angle();
				pos = n.get_pos();
			}
			else if (axiom[i] == 'F') {
				++segment_count;
				if (segment_count >= num_parts_step) {
					part_width /= 1.1f;
					segment_count = 0;
				}

				paint_mat = brown_mat;
				for (unsigned int j = i + 1; j < axiom.size(); ++j) {
					if (axiom[j] == ']') {
						paint_mat = green_mat;
					}
					else if (axiom[j] == 'F') {
						break;
					}
				}

				if (is3D) {
					pos = gen_part(pos, z_angle, y_angle);
				}
				else {
					pos = gen_part(pos, z_angle);
				}

			}
			else if (axiom[i] == 'A') {
				pos = gen_part(pos, z_angle);
			}
			else if (axiom[i] == 'B') {
				pos = gen_part(pos, z_angle);
			}
		}
		printf("\n Pos = %f , %f , %f \n", pos.x(), pos.y(), pos.z());
	}

	int total_parts() {
		int count = 0;
		for (unsigned int i = 0; i < axioms.size(); ++i) {
			if (axioms[i] == 'F' || axioms[i] == 'A' || axioms[i] == 'B') {
				++count;
			}
		}
		return count;
	}

	vec3 gen_part(vec3 start_pos, float z_angle, float y_angle = 0.0f) {
		vec3 mid_pos;
		vec3 end_pos;
		mid_pos.x() = start_pos.x() + part_length * cos((z_angle + 90.0f) * math_PI / 180.0f);
		mid_pos.y() = start_pos.y() + part_length * sin((z_angle + 90.0f) * math_PI / 180.0f);
		end_pos.x() = start_pos.x() + part_length * 2.0f * cos((90.0f + z_angle) * math_PI / 180.0f);
		end_pos.y() = start_pos.y() + part_length * 2.0f * sin((90.0f + z_angle) * math_PI / 180.0f);

		if (total_y < end_pos.y()) {
			total_y = end_pos.y() + part_length;
		}

		printf("%f = part length \n", part_length);
		printf("%f = part width \n", part_width);
		printf("%f =  total y \n", total_y);

		mat4t mtw1;
		mat4t mtw2;

		mtw1.loadIdentity();
		mtw1.rotate(y_angle, 0.0f, 1.0f, 0.0f);

		mtw2.loadIdentity();
		mtw2.translate(mid_pos);
		mtw2.rotate(z_angle, 0.0f, 0.0f, 1.0f);

		mtw2 = mtw2*mtw1;

		if (selected_file_idx < 6) {
			mat4t mtw;
			mtw.loadIdentity();
			mtw.rotate(90.0f, 1.0f, 0.0f, 0.0f);
			zcylinder cyl = zcylinder(vec3(0), part_width, part_length);
			mesh_cylinder *cylinder = new mesh_cylinder(cyl, mtw*mtw2);

			scene_node *node = new scene_node();
			app_scene->add_child(node);
			app_scene->add_mesh_instance(new mesh_instance(node, cylinder, paint_mat));
		}
		else {
			mesh_box *box = new mesh_box(vec3(part_width, part_length, part_width), mtw2);
			scene_node *node = new scene_node();
			app_scene->add_child(node);
			app_scene->add_mesh_instance(new mesh_instance(node, box, red_mat));
		}

		return end_pos;
	}

	//called every frame to draw
	void draw_world(int x, int y, int w, int h) {
		input_manager();

		app_scene->begin_render(w, h);

		mat4t &camera_to_world = camera->get_node()->access_nodeToParent();
		mouse_controller.update(camera_to_world);

		app_scene->update(1.0f / 30.0f);

		app_scene->render((float)w / h);
	}

	//reloads axiom data to new step
	void reload() {
		int count = curr_step;
		
		//axioms.reset();
		axioms.resize(inits.size());

		for (unsigned int i = 0; i < inits.size(); ++i) {
			axioms[i] = inits[i];
		}
		printf("\n step = %i \n", count);
		for (unsigned int i = 0; i < count; ++i) {
			dynarray<char> new_axiom;
			for (unsigned int k = 0; k < axioms.size(); ++k) {

				//printf("\n k = %i \n", k);
				if (check_array(axioms[k], vars)) {
					for (unsigned int j = 0; j < rules[axioms[k]].size(); ++j) {
						new_axiom.push_back(rules[axioms[k]][j]);
					}
				}
				else {
					new_axiom.push_back(axioms[k]);
				}
			}
			axioms.resize(new_axiom.size());
			for (unsigned int k = 0; k < new_axiom.size(); ++k) {
				axioms[k] = new_axiom[k];
			}
		}
		gen_mesh();
	}

	//check if char is in array
	bool check_array(char c, dynarray<char> arr) {
		for (unsigned int i = 0; i < arr.size(); ++i) {
			if (arr[i] == c) {
				return true;
			}
		}
		return false;
	}

	//manages input keys
	void input_manager() {
		if (is_key_going_down(key_up)) {
			curr_step++;
			if (curr_step >= max_step) {
				curr_step = min_step;
			}
			reload();
		}

		if (is_key_going_down(key_down)) {
			curr_step--;
			if (curr_step < min_step) {
				curr_step = max_step;
			}
			reload();
		}

		if (is_key_going_down(key_right)) {
			curr_file = ++curr_file % max_file;
			open_file(curr_file);

			if (curr_file == 6) {
				z_angle_step = 90.0f;
			}
			else if (curr_file == 7) {
				z_angle_step = 60.0f;
			}
			else {
				z_angle_step = 25.0f;
			}

			reload();
		}

		if (is_key_going_down(key_left)) {
			if (--curr_file < 0) {
				curr_file = max_file - 1;
			}
			open_file(curr_file);

			if (curr_file == 6) {
				z_angle_step = 90.0f;
			}
			else if (curr_file == 7) {
				z_angle_step = 60.0f;
			}
			else {
				z_angle_step = 25.0f;
			}

			reload();
		}

		if (is_key_going_down('Z')) {
			if (curr_file >= 6) {
				return;
			}

			if (z_angle_step < max_angle_z) {
				z_angle_step += 1.0f;
				reload();
			}
		}

		if (is_key_going_down('X')) {
			printf("page down %f", 0.0f);
			if (curr_file >= 6) {
				return;
			}

			printf("page down %f", 1.0f);
			if (z_angle_step > min_angle_z) {
				printf("page down %f", 2.0f);
				z_angle_step -= 1.0f;
				reload();
			}
		}

		//toggle 3d (depth of the cylinder stacking) 3d not attaching to tree branch properly
		if (is_key_going_down(key_tab)) {
			is3D = !is3D;
			reload();
		}

		// movement controls
		if (is_key_down(key_w)) {
			camera->get_node()->translate(vec3(0.0f, 0.0f, -1.0f));
		}

		if (is_key_down(key_s)) {
			camera->get_node()->translate(vec3(0.0f, 0.0f, 1.0f));
		}

		if (is_key_down(key_a)) {
			camera->get_node()->translate(vec3(-1.0f, 0.0f, 0.0f));
		}

		if (is_key_down(key_d)) {
			camera->get_node()->translate(vec3(1.0f, 0.0f, 0.0f));
		}

		if (is_key_down(key_space)) {
			camera->get_node()->translate(vec3(0.0f, 1.0f, 0.0f));
		}

		if (is_key_down(key_ctrl)) {
			camera->get_node()->translate(vec3(0.0f, -1.0f, 0.0f));
		}
	}
  };
}
