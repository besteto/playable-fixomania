#include "W4Framework.h"

W4_USE_UNSTRICT_INTERFACE

struct CarPart
{
    CarPart() = default;
    CarPart(sptr<Node> root, const std::string& name, const sptr<MaterialInst>& mat)
    {
        W4_LOG_DEBUG("create part : %s", name.c_str());
        if(name.find("::") != std::string::npos)
        {
           sptr<Node> current_root = root;
           sptr<Node> current_node;

           for(auto const& path : utils::split(name, "::"))
           {
                 current_node = current_root->getChild<Node>(path);
                 current_root = current_node;
           }
           m_mesh = current_node->as<Mesh>();
        }
        else
            m_mesh = root->getChild<Mesh>(name);

        m_mesh->setMaterialInst(mat);
    }
    void show() {m_mesh->setEnabled(true);}
    void hide() {m_mesh->setEnabled(false);}
    void setMat(const sptr<MaterialInst> &mat) {m_mesh->setMaterialInst(mat);}

    static  sptr<CarPart>  create(sptr<Node> root, const std::string& name, sptr<MaterialInst> mat)
    {
        m_parts[name] = std::make_shared<CarPart>(root, name, mat);
        return m_parts[name];
    }
    static sptr<CarPart> get(const std::string& name )
    {
        return m_parts[name];
    }
  private:
    sptr<Mesh>  m_mesh;
    static std::unordered_map<std::string, sptr<CarPart>> m_parts;
};

std::unordered_map<std::string, sptr<CarPart>> CarPart::m_parts;






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Fixomania : public IGame
{
public:

    void onConfig() override
    {
        Game::Config.VFSClean = true;
        Game::Config.UseSimpleInput = true;
        Game::Config.EnableFrustumCulling = true;
    }

    void onStart() override
    {
        setupCamera();
        setupScene();

        isMove = true;
        setGUI();
        createCallbacks();
    }

    void setupScene() {
        auto root = Asset::load(core::Path("resources/assets/car", "car.asset"))->getRoot();
        root->log();

        m_car = root->getChild<Node>("car");

        createMaterials(root->getChild<Node>("lightDirection_01")->getWorldTranslation(), root->getChild<Node>("lightDirection_02")->getWorldTranslation());
        createParts(m_car);

        m_root = w4::make::sptr<RootNode>("root");
        m_root->addChild(root);

        auto background = Mesh::create::plane({1080.f, 1920.f});
            background->setLocalTranslation({0.0f, 35.0f, 590.f});
            background->setWorldRotation(Rotator(0.11f, 0.f, 0.f));
            background->setMaterialInst(m_mats["background"]);
            m_root->addChild(background);

        Render::getPass(0)->setRoot(m_root);
    }

    void setupCamera() {
        auto cam = Render::getScreenCamera();
            cam->setWorldTranslation({0.f, 180.f, -725.f});
            cam->setFov(44.f);
            cam->setAspect(1080.f / 1920.f);
            cam->setFar(10000.f);
            cam->setClearColor(math::vec4(.38f, .82f, .98f, 1.f));
            cam->setClearMask(ClearMask::Color | ClearMask::Depth );
            cam->setWorldRotation(Rotator(0.11f, 0.f, 0.f));
    }

    sptr<gui::Image> createButton(
        sptr<gui::Widget> root,
        const char* image,
        uint height, uint width, int posx,  int posy,
        gui::VerticalAlign va, gui::HorizontalAlign ha,
        const std::function<void(void)>& tapCallback)
    {
        auto widget = gui::createWidget<gui::Image>(root, image, width, height, posx, posy, "button");
             widget->setVerticalAlign(va);
             widget->setHorizontalAlign(ha);
             widget->onTap(tapCallback);
        return widget;
    }

    void  setGUI()
    {
        using vA = gui::VerticalAlign;
        using hA = gui::HorizontalAlign;

        gui::setVirtualResolution({1080, 1920});
        sptr<gui::Widget> widget = gui::createWidget<gui::Widget>(nullptr, "widget");

        imageCurrentDiscount = createButton(widget, m_images[0].c_str(),500, 1000, 40, 250, vA::Top, hA::Left, [this, widget](){});

        createButton(widget, "resources/ui/color_1.png",176, 250, 150, 1700, vA::Center, hA::Center, [this, widget]()
        {
            changeColor(math::vec4(1.f, .0f, .0f, 1.f));
        });
        createButton(widget,"resources/ui/color_2.png", 176, 250, 400, 1700, vA::Center, hA::Center, [this, widget]()
        {
            changeColor(math::vec4(.38f, .82f, .98f, 1.f));
        });
        createButton(widget,"resources/ui/color_3.png", 176, 250, 650, 1700, vA::Center, hA::Center, [this, widget]()
        {
            changeColor(math::vec4(1.f, 1.f, 1.f, 1.f));
        });
        createButton(widget,"resources/ui/color_4.png", 176, 250, 900, 1700, vA::Center, hA::Center, [this, widget]()
        {
            changeColor(math::vec4(.38f, .38f, .38f, 1.f));
        });

        createButton(widget,"resources/ui/st_1_r.png", 100, 100, 930, 1400, vA::Center, hA::Center, [this, widget]()
        {
            isClockwise = false;
        });
        createButton(widget,"resources/ui/st_1_l.png", 100, 100, 150, 1400, vA::Center, hA::Center, [this, widget]()
        {
            isClockwise = true;
        });

        imageReplay = createButton(widget,"resources/ui/play_again_1_400.png", 150, 600, 540, 1500, vA::Center, hA::Center, [this, widget]()
        {
            OnReplay();
        });

        imageReplay->setVisible(false);

    }

    void changeColor(math::vec4 carColor) {
        m_mats["left_door_bad"]->setParam("color", carColor);
        m_mats["body"]->setParam("color", carColor);
        m_mats["bad_body"]->setParam("color", carColor);
    }

    void createMaterials(math::vec3 lightPos1, math::vec3 lightPos2)
    {
        W4_LOG_INFO("so the light_01 pos is %f, %f, %f", lightPos1.x, lightPos1.y, lightPos1.z);
        W4_LOG_INFO("so the light_02 pos is %f, %f, %f", lightPos2.x, lightPos2.y, lightPos2.z);

        auto cubemap = Cubemap::load({
             "resources/textures/Garage_cubeCam_3.png",
             "resources/textures/Garage_cubeCam_4.png",
             "resources/textures/Garage_cubeCam_6.png",
             "resources/textures/Garage_cubeCam_5.png",
             "resources/textures/Garage_cubeCam_1.png",
             "resources/textures/Garage_cubeCam_2.png",});

        auto glassColor = math::vec4(.16f, 0.19f, 0.16f, 1.f);
        auto carColor = math::vec4(.38f, .82f, .98f, 1.f);

        m_mats["podium"] = Material::get("resources/materials/podium.mat")->createInstance();
            m_mats["podium"]->setParam("lightPos", lightPos1);
            m_mats["podium"]->setParam("reflectCoef", 0.2f);
            m_mats["podium"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);
            m_mats["podium"] ->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/PodiumDiffuse.png"));
            m_mats["podium"]->setTexture(TextureId::TEXTURE_1, Texture::get("resources/textures/PodiumSpecular.png"));

        m_mats["body"] = Material::get("resources/materials/body.mat")->createInstance();
            m_mats["body"]->setParam("color", carColor);
            m_mats["body"]->setParam("lightPos", lightPos1);
            m_mats["body"]->setParam("reflectCoef", 0.2f);
            m_mats["body"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);

        m_mats["bad_body"] = Material::get("resources/materials/bad_body.mat")->createInstance();
            m_mats["bad_body"]->setParam("color", carColor);
            m_mats["bad_body"]->setParam("lightPos", lightPos1);
            m_mats["bad_body"]->setParam("reflectCoef", 0.2f);
            m_mats["bad_body"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);

        m_mats["glass"] = Material::get("resources/materials/body.mat")->createInstance();
            m_mats["glass"]->setParam("color", glassColor);
            m_mats["glass"]->setParam("lightPos", lightPos1);
            m_mats["glass"]->setParam("reflectCoef", 0.5f);
            m_mats["glass"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);

        m_mats["glass_side_bad"] = Material::get("resources/materials/bad_body.mat")->createInstance();
            m_mats["glass_side_bad"]->setParam("color", glassColor);
            m_mats["glass_side_bad"]->setParam("lightPos", lightPos1);
            m_mats["glass_side_bad"]->setParam("reflectCoef", 0.5f);
            m_mats["glass_side_bad"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);

        m_mats["glass_o"] = Material::get("resources/materials/glass.mat")->createInstance();
            m_mats["glass_o"]->setParam("color", glassColor);
            m_mats["glass_o"]->setParam("lightPos", lightPos1);
            m_mats["glass_o"]->setParam("opacity", m_opacity/100.f);
//            m_mats["glass_o"]->setBlendFunc(render::BlendFactor::SRC_ALPHA, render::BlendFactor::ONE_MINUS_SRC_ALPHA);
            m_mats["glass_o"]->enableBlending(true);

        m_mats["front_glass_bad"] = Material::get("resources/materials/texture_wlight.mat")->createInstance();
            m_mats["front_glass_bad"]->setParam("color", glassColor);
            m_mats["front_glass_bad"]->setParam("lightPos", lightPos1);
            m_mats["front_glass_bad"]->setParam("reflectCoef", 0.5f);
            m_mats["front_glass_bad"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);
            m_mats["front_glass_bad"]->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/CarBadShader.png"));
        m_mats["left_door_bad"] = Material::get("resources/materials/texture_wlight.mat")->createInstance();
            m_mats["left_door_bad"]->setParam("color", carColor);
            m_mats["left_door_bad"]->setParam("lightPos", lightPos1);
            m_mats["left_door_bad"]->setParam("reflectCoef", 0.2f);
            m_mats["left_door_bad"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);
            m_mats["left_door_bad"]->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/CarBadShader.png"));

        m_mats["main_mat"] = Material::get("resources/materials/complex.mat")->createInstance();
            m_mats["main_mat"]->setParam("lightPos", lightPos1);
            m_mats["main_mat"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);
            m_mats["main_mat"]->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/CarDiffuse.png"));
            m_mats["main_mat"]->setTexture(TextureId::TEXTURE_1, Texture::get("resources/textures/CarSpecular.png"));
            m_mats["main_mat"] ->setTexture(TextureId::TEXTURE_3, Texture::get("resources/textures/CarRoughness.png"));

        m_mats["main_mat_bad"] = Material::get("resources/materials/bad_complex.mat")->createInstance();
            m_mats["main_mat_bad"]->setParam("lightPos", lightPos1);
            m_mats["main_mat_bad"]->setCubemap(CubemapId::CUBEMAP_0, cubemap);
            m_mats["main_mat_bad"]->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/CarDiffuse.png"));
            m_mats["main_mat_bad"]->setTexture(TextureId::TEXTURE_1, Texture::get("resources/textures/CarSpecular.png"));
            m_mats["main_mat_bad"] ->setTexture(TextureId::TEXTURE_3, Texture::get("resources/textures/CarRoughness.png"));

        m_mats["background"] = Material::get("resources/materials/background.mat")->createInstance();
            m_mats["background"] ->setTexture(TextureId::TEXTURE_0, Texture::get("resources/textures/Background.jpg"));
            m_mats["background"] ->setTexture(TextureId::TEXTURE_1, Texture::get("resources/textures/BackgroundDOF_light.png"));
            m_mats["background"] ->setTexture(TextureId::TEXTURE_2, Texture::get("resources/textures/BackgroundDOF_specLight.png"));
    }

    void createParts(sptr<Node> root)
    {
        CarPart::create(root, "podium",                 m_mats["podium"])           -> show();

        CarPart::create(root, "glass_front_bad",        m_mats["front_glass_bad"])  -> show();
        CarPart::create(root, "glass_front_fixed",      m_mats["glass"])            -> hide();
        CarPart::create(root, "glass_back",             m_mats["main_mat"])         -> show();
        CarPart::create(root, "glass_side",             m_mats["glass"])            -> show();
        CarPart::create(root, "glass_side_bad",         m_mats["glass_side_bad"])   -> show();
        CarPart::create(root, "glass_side_fixed",       m_mats["glass"])            -> hide();
        CarPart::create(root, "glass_headlights",       m_mats["glass_o"])          -> show();
        CarPart::create(root, "glass_taillights_right", m_mats["glass_o"])          -> show();
        CarPart::create(root, "glass_taillights_left",  m_mats["glass_o"])          -> show();
        CarPart::create(root, "headlight_fixed::glass_headlight_fixed",  m_mats["glass_o"])       -> hide();
        CarPart::create(root, "headlight_fixed::headlight_left_fixed",        m_mats["main_mat"]) -> hide();
        CarPart::create(root, "interior",               m_mats["main_mat"])         -> show();
        CarPart::create(root, "front_wheel_bad",        m_mats["main_mat_bad"])     -> show();
        CarPart::create(root, "front_wheel_fixed",      m_mats["main_mat"])         -> hide();
        CarPart::create(root, "rear_wheel_bad",         m_mats["main_mat_bad"])     -> show();
        CarPart::create(root, "rear_wheel_fixed",       m_mats["main_mat"])         -> hide();
        CarPart::create(root, "headlight_bad",          m_mats["main_mat_bad"])     -> show();
        CarPart::create(root, "body_left_door",         m_mats["left_door_bad"])    -> show();
        CarPart::create(root, "body1",                  m_mats["body"])             -> show();
        CarPart::create(root, "bamper_fixed::body_bamper_fixed",   m_mats["body"])  -> hide();
        CarPart::create(root, "bamper_fixed::grid_fixed",   m_mats["main_mat"])     -> hide();
        CarPart::create(root, "bamper_bad::body_bamper_bad",   m_mats["bad_body"])  -> show();
        CarPart::create(root, "bamper_bad::grid_bad",   m_mats["main_mat_bad"])     -> show();
    }

    void createCallbacks()
    {
        auto& cc1 = m_car->getChild<Mesh>("glass_side_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc1.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("glass_side_bad")->hide();
                                         CarPart::get("glass_side_fixed")->show();
                                         addDiscount();
                                     });

        auto& cc2 = m_car->getChild<Mesh>("front_wheel_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc2.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("front_wheel_bad")->hide();
                                         CarPart::get("front_wheel_fixed")->show();
                                         addDiscount();
                                     });

        auto& cc3 = m_car->getChild<Mesh>("rear_wheel_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc3.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("rear_wheel_bad")->hide();
                                         CarPart::get("rear_wheel_fixed")->show();
                                         addDiscount();
                                     });


        auto& cc4 = m_car->getChild<Mesh>("headlight_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc4.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("headlight_bad")->hide();
                                         CarPart::get("headlight_fixed::headlight_left_fixed")->show();
                                         CarPart::get("headlight_fixed::glass_headlight_fixed")->show();
                                         addDiscount();
                                     });

        auto& cc5 = m_car->getChild<Node>("bamper_bad")->getChild<Mesh>("body_bamper_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc5.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("bamper_fixed::body_bamper_fixed")->show();
                                         CarPart::get("bamper_fixed::grid_fixed")->show();
                                         CarPart::get("bamper_bad::body_bamper_bad")->hide();
                                         CarPart::get("bamper_bad::grid_bad")->hide();
                                         addDiscount();
                                     });
        auto& cc6 = m_car->getChild<Node>("bamper_bad")->getChild<Mesh>("grid_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc6.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("bamper_fixed::body_bamper_fixed")->show();
                                         CarPart::get("bamper_fixed::grid_fixed")->show();
                                         CarPart::get("bamper_bad::body_bamper_bad")->hide();
                                         CarPart::get("bamper_bad::grid_bad")->hide();
                                         addDiscount();
                                     });

        auto& cc7 = m_car->getChild<Mesh>("body_left_door")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc7.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("body_left_door")->setMat(m_mats["body"]);
                                         addDiscount();
                                     });


        auto& cc8 = m_car->getChild<Mesh>("glass_front_bad")->addComponent<render::RaycastComponent>("unnamed", render::RaycastComponent::Geometry::Mesh);
        cc8.setScreenRaycastCallback(render::RayCastEvent::Down, Render::getPass(0),
                                     [this](const render::CollidingInfo& info)
                                     {
                                         CarPart::get("glass_front_bad")->hide();
                                         CarPart::get("glass_front_fixed")->show();
                                         addDiscount();
                                     });

    }

    void addDiscount() {

        if (m_discontPoints < 7) {
            ++m_discontPoints;
            W4_LOG_INFO("Your discount now is %d percent!", m_discontPoints*5);
            imageCurrentDiscount->setImage(m_images[m_discontPoints]);
        }

        if (m_discontPoints == 7){OnWin(m_discontPoints);}
    }

   void OnWin(int numb)
    {
        imageReplay->setVisible(true);
    }
    void OnReplay()
    {
        m_discontPoints = 0;
        imageReplay->setVisible(false);
        imageCurrentDiscount->setImage(m_images[0]);
        BrokeThemAll();
    }

    void BrokeThemAll() {
        CarPart::get("glass_side_bad")->show();
        CarPart::get("glass_side_fixed")->hide();

        CarPart::get("front_wheel_bad")->show();
        CarPart::get("front_wheel_fixed")->hide();

        CarPart::get("rear_wheel_bad")->show();
        CarPart::get("rear_wheel_fixed")->hide();

        CarPart::get("headlight_bad")->show();
        CarPart::get("headlight_fixed::headlight_left_fixed")->hide();
        CarPart::get("headlight_fixed::glass_headlight_fixed")->hide();

        CarPart::get("bamper_fixed::body_bamper_fixed")->hide();
        CarPart::get("bamper_fixed::grid_fixed")->hide();
        CarPart::get("bamper_bad::body_bamper_bad")->show();
        CarPart::get("bamper_bad::grid_bad")->show();

        CarPart::get("body_left_door")->setMat(m_mats["body"]);

        CarPart::get("glass_front_bad")->show();
        CarPart::get("glass_front_fixed")->hide();
    }

    void OnTap(event::Touch::Begin::cref e)
    {

    }

    void onKey(const event::Keyboard::Down& evt) override

    {
        switch(evt.key)
        {
            case event::Keyboard::Key::Space:
                isMove = !isMove;
                break;
            case event::Keyboard::Key::Digit1:
                CarPart::get("glass_side_bad")->hide();
                CarPart::get("glass_side_fixed")->show();
                addDiscount();
                break;
            case event::Keyboard::Key::Digit2:
                CarPart::get("front_wheel_bad")->hide();
                CarPart::get("front_wheel_fixed")->show();
                addDiscount();
                break;
            case event::Keyboard::Key::Digit3:
                CarPart::get("rear_wheel_bad")->hide();
                CarPart::get("rear_wheel_fixed")->show();
                addDiscount();
                break;
            case event::Keyboard::Key::Digit4:
                CarPart::get("headlight_bad")->hide();
                CarPart::get("headlight_fixed::headlight_left_fixed")->show();
                CarPart::get("headlight_fixed::glass_headlight_fixed")->show();
                addDiscount();
                break;
            case event::Keyboard::Key::Digit5:
                CarPart::get("bamper_fixed::body_bamper_fixed")->show();
                CarPart::get("bamper_fixed::grid_fixed")->show();
                CarPart::get("bamper_bad::body_bamper_bad")->hide();
                CarPart::get("bamper_bad::grid_bad")->hide();
                addDiscount();
                break;
            case event::Keyboard::Key::Digit6:
                CarPart::get("body_left_door")->setMat(m_mats["body"]);
                addDiscount();
                break;
            case event::Keyboard::Key::Digit7:
                CarPart::get("glass_front_bad")->hide();
                CarPart::get("glass_front_fixed")->show();
                addDiscount();
                break;
            default:
                break;
        }
    }

    void OnSwipe(event::Swipe::cref e)
    {

    }

    void onUpdate(float dt) override
    {
        if(isMove)
        {
            auto da = PI / 180.f / 4.f;
            if (isClockwise) {da = -da;}

            auto rot = m_car->getLocalRotation();
            rot -= Rotator(0.f, da, 0.f);
            m_car->setLocalRotation(rot);
        }
    }


private:

    sptr<RootNode>  m_root;
    sptr<Node>      m_car;

    std::unordered_map<std::string, sptr<CarPart>>      m_parts;
    std::unordered_map<std::string, sptr<MaterialInst>> m_mats;
    std::string m_images[8] = {
        "resources/ui/0_panel.png",
        "resources/ui/5_panel.png",
        "resources/ui/10_panel.png",
        "resources/ui/15_panel.png",
        "resources/ui/20_panel.png",
        "resources/ui/25_panel.png",
        "resources/ui/30_panel.png",
        "resources/ui/35_panel.png"
    };

    bool isMove;
    bool isClockwise;
    size_t m_discontPoints = 0;
    float m_opacity = 0.7f;
    sptr<gui::Image> imageCurrentDiscount;
    sptr<gui::Image> imageReplay;
};

W4_RUN(Fixomania)
