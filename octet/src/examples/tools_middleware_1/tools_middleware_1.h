////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#define MAX_BALLS 10
namespace octet {
  /// Scene containing a box with octet.
  class tools_middleware_1 : public app {
	  // scene for drawing box
      const float mathPI = 3.14159;
	  ref<visual_scene> app_scene;
	  mouse_look mouse_controller;
	  ref<camera_instance> camera;

	  //meshes and variables
	  int ball_idx = 0;
	  int wallColor = 0;
	  mesh_instance *balls[MAX_BALLS];
	  mesh_instance *wall;
	  mesh_instance *ground;

	  //bullet physic vars
	  btDiscreteDynamicsWorld *btWorld;

  public:
	  tools_middleware_1(int argc, char **argv) : app(argc, argv) {
	  }

	  ~tools_middleware_1() {
	  }

	  //uses keyboard input for the demo
	  void apply_input() {
		  if (is_key_down(key_left) || is_key_down(key_a)) {
			  camera->get_node()->translate(vec3(-0.5, 0, 0));
		  }
		  else if (is_key_down(key_right) || is_key_down(key_d)) {
			  camera->get_node()->translate(vec3(0.5, 0, 0));
		  }
		  else if (is_key_down(key_up) || is_key_down(key_w)) {
			  camera->get_node()->translate(vec3(0, 0, -0.5));
		  }
		  else if (is_key_down(key_down) || is_key_down(key_s)) {
			  camera->get_node()->translate(vec3(0, 0, 0.5));
		  }
		  else if (is_key_down(key_shift)) {
			  camera->get_node()->translate(vec3(0, -0.5, 0));
		  }
		  else if (is_key_down(key_ctrl)) {
			  camera->get_node()->translate(vec3(0, 0.5, 0));
		  }
		  else if (is_key_going_down(key_space)) { //shoot
			  if (ball_idx < MAX_BALLS) {
				  balls[ball_idx]->get_node()->set_position(vec3(camera->get_node()->get_position()));
				  balls[ball_idx]->get_node()->get_rigid_body()->forceActivationState(ACTIVE_TAG);
				  balls[ball_idx]->set_flags(balls[ball_idx]->get_flags() ^ 2); //only disables rendering not physics
				  balls[ball_idx]->get_node()->apply_central_force((-camera->get_node()->get_z())*800.0f);
				  ball_idx++;
			  }
		  }

	  }

	  ///Use bulletp physics to check for collisions
	  void check_collision() {

		  int num_manifolds = btWorld->getDispatcher()->getNumManifolds();
		  
		  for (unsigned int i = 0; i < num_manifolds; i++) {
			  btPersistentManifold *manifold = btWorld->getDispatcher()->getManifoldByIndexInternal(i);
			  int objA = manifold->getBody0()->getUserIndex();
			  int objB = manifold->getBody1()->getUserIndex();
			  int wallIdx = wall->get_node()->get_rigid_body()->getUserIndex();
			  if (objA == wallIdx || objB == wallIdx) {
				  for (int j = 0; j < MAX_BALLS; j++) {
					  int ballIdx = balls[j]->get_node()->get_rigid_body()->getUserIndex();
					  if (objA == ballIdx || objB == ballIdx) {
						  wallColor++;
						  balls[j]->set_flags(balls[ball_idx]->get_flags() ^ 2);
						  material *red = new material(vec4(1, 0, 0, 1));
						  material *green = new material(vec4(0, 1, 0, 1));
						  material *blue = new material(vec4(0, 0, 1, 1));
						  if (wallColor > 2) wallColor = 0;
						  if (wallColor == 0) balls[j]->set_material(red);
						  if (wallColor == 1) balls[j]->set_material(green);
						  if (wallColor == 2) balls[j]->set_material(blue);
						  balls[j]->set_flags(balls[ball_idx]->get_flags() ^ 2);
					  }
				  }
			  }
		  }
	  }

	  /// this is called once OpenGL is initialized
	  void app_init() {
		  //printf("\n Wall = %f , %f, %f  -- Ground = %f, %f, %f \n", wall->get_node()->get_position().x(), wall->get_node()->get_position().y(), wall->get_node()->get_position().z(), ground->get_node()->get_position().x(), ground->get_node()->get_position().y(), ground->get_node()->get_position().z());
		  //printf("\n Cam = %f , %f, %f \n", camera->get_node()->get_position().x(), camera->get_node()->get_position().y(), camera->get_node()->get_position().z());


		  //init scene and controls
		  app_scene = new visual_scene();
		  app_scene->create_default_camera_and_lights();
		  btWorld = app_scene->get_btworld();
		  camera = app_scene->get_camera_instance(0);
		  camera->get_node()->translate(vec3(0, 20, 0));
		  camera->set_far_plane(500.0f);
		  mouse_controller.init(this, 100.0f / 360.0f, false);

		  //init materials for objects
		  material *red = new material(vec4(1, 0, 0, 1));
		  material *green = new material(vec4(0, 1, 0, 1));
		  material *blue = new material(vec4(0, 0, 1, 1));
		  mat4t mat;

		  // ground
		  mat.loadIdentity();
		  mat.translate(0, -6, 0);
		  ground = app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);

		  //backboard
		  mat.loadIdentity();
		  mat.translate(0, 6, -30);
		  wall = app_scene->add_shape(mat, new mesh_box(vec3(20, 20, 10)), green, false);
		  btTransform wallTrans = btTransform::getIdentity();
		  wallTrans.setOrigin(btVector3(btScalar(0.0f), btScalar(-10.0f), btScalar(0.0f)));

		  ////Ring shape torus - attempted
		  //mat.loadIdentity();
		  //mat.translate( 3, 6, 0);
		  //app_scene->add_shape(mat, new mesh_cylinder(torus(vec3(0, 0, 0), 2, 4)), blue, true);

		  mesh_instance *hoopF;
		  mesh_instance *hoopB;
		  mesh_instance *hoopR;
		  mesh_instance *hoopL;

		  //hoop front
		  mat.loadIdentity();
		  mat.translate(0, 10, -14);
		  hoopF = app_scene->add_shape(mat, new mesh_box(vec3(4, 0.5f, 0.5f)), blue, false);
		  //tried to use constraints but fixed contraints have springy ness..
		  //wallTrans.setOrigin(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  //transF.setOrigin(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(-10.5f)));
		  //btGeneric6DofConstraint *hoop1 = new btGeneric6DofConstraint(*(wall->get_node()->get_rigid_body()), *(hoopF->get_node()->get_rigid_body()), wallTrans, transF, true);
		  //hoop1->setLinearLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  //hoop1->setLinearUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  //hoop1->setAngularLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  //hoop1->setAngularUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  //app_scene->add_constraint(hoop1);

		  //hoop back
		  mat.loadIdentity();
		  mat.translate(0, 10, -19);
		  hoopB = app_scene->add_shape(mat, new mesh_box(vec3(4, 0.5f, 0.5f)), blue, false);

		  //hoop left
		  mat.loadIdentity();
		  mat.translate(-3.5f, 10, -16.5f);
		  hoopL = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 0.5f, 4)), blue, false);

		  //hoop right
		  mat.loadIdentity();
		  mat.translate(3.5f, 10, -16.5f);
		  hoopR = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 0.5f, 4)), blue, false);

		  //net sides front - these are cubes attached with spring constraints to the hoop (which is static)
		  mesh_instance *netSideF;
		  mesh_instance *netSideB;
		  mesh_instance *netSideL;
		  mesh_instance *netSideR;
		  btTransform transFBRL = btTransform::getIdentity();
		  btTransform transNet = btTransform::getIdentity();
		  transFBRL.setOrigin(btVector3(btScalar(0.0f), btScalar(5.5f), btScalar(0.0f)));
		  transNet.setOrigin(btVector3(btScalar(0.0f), btScalar(-0.5f), btScalar(-10.5f)));

		  mat.loadIdentity();
		  mat.translate(0, 6, -14);
		  netSideF = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  //netSideF->get_node()->set_mass(5.0f, vec3(0.0f,0.0f,0.0f)); -- Crashes???
		  btGeneric6DofSpringConstraint *netSpringF = new btGeneric6DofSpringConstraint(*(hoopF->get_node()->get_rigid_body()), *(netSideF->get_node()->get_rigid_body()), transNet, transFBRL, true);
		  //Relative to origin,, how far up / down it will go
		  netSpringF->setLinearLowerLimit(btVector3(btScalar(0.0f), btScalar(-3.0f), btScalar(0.0f)));
		  netSpringF->setLinearUpperLimit(btVector3(btScalar(0.0f), btScalar(-1.0f), btScalar(0.0f)));
		  //Locking rotation
		  netSpringF->setAngularLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  netSpringF->setAngularUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  app_scene->add_constraint(netSpringF);
		  netSpringF->setDbgDrawSize(btScalar(5.f));
		  netSpringF->enableSpring(1, true); //true should mean start at rest...
		  netSpringF->setStiffness(1, 150.0f); //defualt is 0.0f, how much bouncyness, / pushback,  1 mean translate in Y axis
		  netSpringF->setDamping(1, 0.01f); //Normalized value, 1 is axis again. How much will is slowed down the spring till at rest
		  //netSpringF->setParam(BT_CONSTRAINT_STOP_CFM, 1.0e-5f, 5);
		  netSpringF->setEquilibriumPoint(); // set current position as equilibrium point, point of rest... doesnt really change anything...

		  mat.loadIdentity();
		  mat.translate(0, 6, -19);
		  netSideB = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btGeneric6DofSpringConstraint *netSpringB = new btGeneric6DofSpringConstraint(*(hoopB->get_node()->get_rigid_body()), *(netSideB->get_node()->get_rigid_body()), transNet, transFBRL, true);
		  //Relative to origin,, how far up / down it will go
		  netSpringB->setLinearLowerLimit(btVector3(btScalar(0.0f), btScalar(-3.0f), btScalar(0.0f)));
		  netSpringB->setLinearUpperLimit(btVector3(btScalar(0.0f), btScalar(-1.0f), btScalar(0.0f)));
		  //Locking rotation
		  netSpringB->setAngularLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  netSpringB->setAngularUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  app_scene->add_constraint(netSpringB);
		  netSpringB->setDbgDrawSize(btScalar(5.f));
		  netSpringB->enableSpring(1, true); //true should mean start at rest...
		  netSpringB->setStiffness(1, 150.0f); //defualt is 0.0f, how much bouncyness, / pushback,  1 mean translate in Y axis
		  netSpringB->setDamping(1, 0.01f); //Normalized value, 1 is axis again. How much will is slowed down the spring till at rest
		  //netSpringB->setParam(BT_CONSTRAINT_STOP_CFM, 1.0e-5f, 5);
		  netSpringB->setEquilibriumPoint(); // set current position as equilibrium point, point of rest... doesnt really change anything...


		  mat.loadIdentity();
		  mat.translate(-3.5f, 6, -16.5f);
		  netSideL = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btGeneric6DofSpringConstraint *netSpringL = new btGeneric6DofSpringConstraint(*(hoopL->get_node()->get_rigid_body()), *(netSideL->get_node()->get_rigid_body()), transNet, transFBRL, true);
		  //Relative to origin,, how far up / down it will go
		  netSpringL->setLinearLowerLimit(btVector3(btScalar(0.0f), btScalar(-3.0f), btScalar(0.0f)));
		  netSpringL->setLinearUpperLimit(btVector3(btScalar(0.0f), btScalar(-1.0f), btScalar(0.0f)));
		  //Locking rotation
		  netSpringL->setAngularLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  netSpringL->setAngularUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  app_scene->add_constraint(netSpringL);
		  netSpringL->setDbgDrawSize(btScalar(5.f));
		  netSpringL->enableSpring(1, true); //true should mean start at rest...
		  netSpringL->setStiffness(1, 150.0f); //defualt is 0.0f, how much bouncyness, / pushback,  1 mean translate in Y axis
		  netSpringL->setDamping(1, 0.01f); //Normalized value, 1 is axis again. How much will is slowed down the spring till at rest
		  //netSpringL->setParam(BT_CONSTRAINT_STOP_CFM, 1.0e-5f, 5);
		  netSpringL->setEquilibriumPoint(); // set current position as equilibrium point, point of rest... doesnt really change anything...


		  mat.loadIdentity();
		  mat.translate(3.5f, 6, -16.5f);
		  netSideR = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btGeneric6DofSpringConstraint *netSpringR = new btGeneric6DofSpringConstraint(*(hoopR->get_node()->get_rigid_body()), *(netSideR->get_node()->get_rigid_body()), transNet, transFBRL, true);
		  //Relative to origin,, how far up / down it will go
		  netSpringR->setLinearLowerLimit(btVector3(btScalar(0.0f), btScalar(-3.0f), btScalar(0.0f)));
		  netSpringR->setLinearUpperLimit(btVector3(btScalar(0.0f), btScalar(-1.0f), btScalar(0.0f)));
		  //Locking rotation
		  netSpringR->setAngularLowerLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  netSpringR->setAngularUpperLimit(btVector3(btScalar(0.0f), btScalar(0.0f), btScalar(0.0f)));
		  app_scene->add_constraint(netSpringR);
		  netSpringR->setDbgDrawSize(btScalar(5.f));
		  netSpringR->enableSpring(1, true); //true should mean start at rest...
		  netSpringR->setStiffness(1, 1000.0f); //defualt is 0.0f, how much bouncyness, / pushback,  1 mean translate in Y axis
		  netSpringR->setDamping(1, 0.01f); //Normalized value 1 is no dampening, 1 is axis again. How much will is slowed down the spring till at rest
		  //netSpringR->setParam(BT_CONSTRAINT_STOP_CFM, 1.0e-5f, 5);
		  netSpringR->setEquilibriumPoint(); // set current position as equilibrium point, point of rest... doesnt really change anything..
		 
		  //Hinge Between springs  objects in a circle
		  btHingeConstraint *netHingeSideL = new btHingeConstraint(*(netSideF->get_node()->get_rigid_body()), *(netSideL->get_node()->get_rigid_body()), btVector3(-0.25f, 0.0f, -0.25f), btVector3(0.0f, 0.0f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeSideL->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeSideL);
		  btHingeConstraint *netHingeSideR = new btHingeConstraint(*(netSideB->get_node()->get_rigid_body()), *(netSideR->get_node()->get_rigid_body()), btVector3(0.25f, 0.0f, 0.25f), btVector3(0.0f, 0.0f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeSideR->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeSideR);
		  btHingeConstraint *netHingeSideB = new btHingeConstraint(*(netSideL->get_node()->get_rigid_body()), *(netSideB->get_node()->get_rigid_body()), btVector3(0.25f, 0.0f, -0.25f), btVector3(0.0f, 0.0f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeSideB->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeSideB);
		  btHingeConstraint *netHingeSideF = new btHingeConstraint(*(netSideR->get_node()->get_rigid_body()), *(netSideF->get_node()->get_rigid_body()), btVector3(-0.25f, 0.0f, 0.25f), btVector3(0.0f, 0.0f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeSideF->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeSideF);

		  //Lower cubes attached with hinges
		  mesh_instance *netBotF;
		  mesh_instance *netBotB;
		  mesh_instance *netBotL;
		  mesh_instance *netBotR;
		  mat.loadIdentity();
		  mat.translate(0.0f, 2.0f, -14.0f);
		  netBotF = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btHingeConstraint *netHingeF = new btHingeConstraint(*(netSideF->get_node()->get_rigid_body()), *(netBotF->get_node()->get_rigid_body()), btVector3(0.0f, -0.5f, 0.0f), btVector3(0.0f, 2.5f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeF->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeF);
		  mat.loadIdentity();
		  mat.translate(0.0f, 2.0f, -19.0f);
		  netBotB = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btHingeConstraint *netHingeB = new btHingeConstraint(*(netSideB->get_node()->get_rigid_body()), *(netBotB->get_node()->get_rigid_body()), btVector3(0.0f, -0.5f, 0.0f), btVector3(0.0f, 2.5f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeB->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeB);
		  mat.loadIdentity();
		  mat.translate(-3.5f, 2.0f, -16.5f);
		  netBotL = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btHingeConstraint *netHingeL = new btHingeConstraint(*(netSideL->get_node()->get_rigid_body()), *(netBotL->get_node()->get_rigid_body()), btVector3(0.0f, -0.5f, 0.0f), btVector3(0.0f, 2.5f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeL->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeL);
		  mat.loadIdentity();
		  mat.translate(3.5f, 2.0f, -16.5f);
		  netBotR = app_scene->add_shape(mat, new mesh_box(vec3(0.5f, 1.0f, 0.5f)), blue, true);
		  btHingeConstraint *netHingeR = new btHingeConstraint(*(netSideR->get_node()->get_rigid_body()), *(netBotR->get_node()->get_rigid_body()), btVector3(0.0f, -0.5f, 0.0f), btVector3(0.0f, 2.5f, 0.0f), btVector3(1, 0, 1), btVector3(1, 0, 1), false);
		  netHingeR->setLimit(-mathPI * 0.2f, mathPI * 0.2f);
		  app_scene->add_constraint(netHingeR);

		  //init ball and disable flag
		  for (int i = 0; i < MAX_BALLS; ++i) {
			  if (i > 0) mat.loadIdentity();
			  mat.translate(0, 20, 0);
			  balls[i] = app_scene->add_shape(mat, new mesh_sphere(vec3(1, 1, 1), 2), red, true);
			  balls[i]->set_flags(balls[i]->get_flags() ^ 2); //only disables rendering not physics
			  balls[i]->get_node()->get_rigid_body()->setActivationState(DISABLE_SIMULATION);
		  }


	  }

	  /// this is called to draw the world
	  void draw_world(int x, int y, int w, int h) {
		  int vx = 0, vy = 0;

		  get_viewport_size(vx, vy);
		  app_scene->begin_render(vx, vy);

		  apply_input();

		  check_collision();

		  mat4t &camera_to_world = camera->get_node()->access_nodeToParent();
		  mouse_controller.update(camera_to_world);

		  // update matrices. assume 30 fps.
		  app_scene->update(1.0f / 30);

		  // draw the scene
		  app_scene->render((float)vx / vy);
		 

	  }
  };
}
