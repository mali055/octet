////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// animation example: Drawing an jointed figure with animation
//
// Level: 2
//
// Demonstrates:
//   Collada meshes
//   Collada nodes
//   Collada animation
//
// note this app is not in the octet namespace as it is not part of octet
// and so we need to use octet:: on several classes.

class engine : public octet::app {
  typedef octet::mat4t mat4t;
  typedef octet::vec4 vec4;
  typedef octet::animation animation;
  typedef octet::scene_node scene_node;

  // named resources loaded from collada file
  octet::resources dict;

  // scene used to draw 3d objects
  octet::ref<octet::scene> app_scene;

  // shaders to draw triangles
  octet::bump_shader object_shader;
  octet::bump_shader skin_shader;

  // helper to rotate camera about scene
  octet::mouse_ball ball;

  // helper for drawing text
  octet::text_overlay overlay;

  // helper for debugging by web browser
  octet::http_server server;


  void load_file(const char *filename) {
    octet::collada_builder builder;
    if (!builder.load_xml(filename)) {
      printf("\nERROR: could not open %s\nThis is likely a problem with paths.", filename);
      return;
    }

    builder.get_resources(dict);
    app_scene = dict.get_scene(builder.get_default_scene());

    assert(app_scene);

    app_scene->create_default_camera_and_lights();

    /*{
      // save the resources as an xml file
      TiXmlDocument doc;
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "");
      doc.LinkEndChild(decl);
      TiXmlElement *root = new TiXmlElement("octet");
      doc.LinkEndChild(root);

      octet::xml_writer xml(root);

      dict.visit(xml);

      doc.SaveFile("animation.xml");
    }*/

    app_scene->play_all_anims(dict);

    if (app_scene->num_camera_instances() != 0) {
      octet::camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      mat4t cameraToWorld = node->get_nodeToParent();
      ball.init(this, cameraToWorld.w().length(), 360.0f);
    }
  }
public:
  // this is called when we construct the class
  engine(int argc, char **argv) : app(argc, argv), ball() {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shaders
    object_shader.init(false);
    skin_shader.init(true);

    const char *filename = 0;

    int selector = 0;
    switch (selector) {
      case 0: filename = "assets/duck_triangulate.dae"; break;
      case 1: filename = "assets/skinning/skin_unrot.dae"; break;
      case 2: filename = "assets/jenga.dae"; break;
      case 3: filename = "assets/duck_ambient.dae"; break;
      case 4: filename = "assets/Laurana50k.dae"; break;
      case 5: filename = "external/Arteria3d/ElvenMale/ElevenMaleKnight_blender.dae"; break;
      case 6: filename = "external/Arteria3d/arteria3d_tropicalpack/flowers/flower%202.dae"; break; 
    }

    load_file(filename);


    overlay.init();
    server.init(&dict);
  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to black
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
    glEnable(GL_DEPTH_TEST);

    // allow alpha blend (transparency when alpha channel is 0)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // poll web server
    server.update();

    // drag and drop file loading
    octet::dynarray<octet::string> &queue = access_load_queue();
    if (queue.size()) {
      // replace scene
      dict.reset();
      octet::string url;
      url.urlencode(queue[0]);
      load_file(url);
      queue.resize(0);
    }

    if (app_scene && app_scene->num_camera_instances()) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      octet::camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      mat4t &cameraToWorld = node->access_nodeToParent();
      ball.update(cameraToWorld);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

      overlay.render(object_shader, skin_shader, vx, vy);
    }
  }
};