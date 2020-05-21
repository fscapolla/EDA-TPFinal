#include "Graphic.h"


Graphic::Graphic()
{
	if (AllegroInit() && ImguiInit())
	{
		this->queue = al_create_event_queue();
		al_register_event_source(this->queue, al_get_display_event_source(display));
		al_register_event_source(this->queue, al_get_mouse_event_source());
		al_register_event_source(this->queue, al_get_keyboard_event_source());

		window_flags |= ImGuiWindowFlags_NoResize;
		InputState = false;
		Error = false;
		close = false;
		EstadoActual = Estado::MainMenu;
	}
	else
	{
		Error = true;
	}
}

Graphic::~Graphic()
{
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	if (!Error)
	{
		al_shutdown_primitives_addon();
		al_uninstall_keyboard();
		al_uninstall_mouse();
		al_shutdown_image_addon();
	}
	if (queue)
		al_destroy_event_queue(queue);
	if (display)
		al_destroy_display(display);
}

bool Graphic::GetError()
{
	return Error;
}

bool Graphic::RunningOne()
{
	if (hayEvento())
	{
		Dispatch();
	}
	return (!close);
}

bool Graphic::hayEvento(void)
{
	bool ret = false;

	while (al_get_next_event(queue, &ev))
	{
		ImGui_ImplAllegro5_ProcessEvent(&ev);
	}

	if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
	{
		EventoActual = Evento::Close;
		ret = true;
	}

	if (print_current_state(EstadoActual))				//Devuelve true si huvo un evento (Usuario presiono un boton)
	{
		ret = true;
	}

	return ret;
}

void Graphic::Dispatch(void)
{
	switch (EventoActual)
	{
	case Evento::Close:
		close = true;
		break;

	case Evento::gotoMainMenu:
		flushVariables();
		break;

	case Evento::StartExtraction:
		InputState = true;

	default:
		break;
	}
}

bool Graphic::print_current_state(Estado CurrentState)
{
	bool userEvento = false;
	switch (CurrentState)
	{
	case Estado::MainMenu:					//En MainMenu apareceran los eventos StartComp StartDecomp 
		if (print_MainMenu() && EventoActual != Evento::gotoMainMenu)
		{
			userEvento = true;
		}
		break;

	case Estado::SelectingBlocks:
		if (print_SelectBlocks())
		{
			userEvento = true;			//se terminaron de seleccionar las imagenes
		}
		break;

	case Estado::Error:
		if (print_Error())
		{
			userEvento = true;			//Se cerro display
		}
		break;

	case Estado::Loading:
		print_Loading();
		break;

	case Estado::InfoReady:
		if (print_Done())
		{
			userEvento = true;
		}
		break;

	default:
		break;
	}

	return userEvento;
}

bool Graphic::print_MainMenu()
{
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	bool eventoMenu = false;


	ImGui::SetNextWindowPos(ImVec2(200, 10));
	ImGui::SetNextWindowSize(ImVec2(600, 150));

	ImGui::Begin("Inserte el camino al directorio con los bloques", 0, window_flags);
	static char paths[MAX_PATH];

	ImGui::InputText("Directorio", paths, sizeof(char) * MAX_PATH);
	if (ImGui::Button("Iniciar"))
	{
		EstadoActual = Estado::SelectingBlocks;
		path.assign(paths);
		directoryName.assign(paths);
		look4BlocksPath();
		eventoMenu = true;
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	al_flip_display();

	return eventoMenu;
}

void Graphic::look4BlocksPath()
{
	fs::path bPath(directoryName);
	if (exists(bPath) && is_directory(bPath))
	{
		for (fs::directory_iterator iterator(bPath); iterator != fs::directory_iterator(); iterator++)
		{
			if (iterator->path().filename().string() == "blockChain.json")
			{
				//saveBlockInfo(iterator->path().string());
				std::cout << iterator->path().string() << std::endl;

			}
		}
	}
	else
	{
		EstadoActual = Estado::Error;
	}
}

void Graphic::saveBlockInfo(std::string path)
{

	std::cout << "SAVE BLOCK INFO" << std::endl;

}

bool Graphic::print_SelectBlocks()
{
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	bool blocksEvento = false;
	int i;


	ImGui::SetNextWindowPos(ImVec2(200, 10));
	ImGui::SetNextWindowSize(ImVec2(600, 650));

	ImGui::Begin("Seleccione bloques", 0, window_flags);

	//Checkbox con imgui
	static bool checks[MAX_BLOCKS] = { false };

	for (i = 0; i < BlocksArr.size(); i++)
	{
		//ImGui::Checkbox(BlocksArr[i].getBlockID().c_str(), &checks[i]);
	}

	if (ImGui::Button("Buscar Info"))
	{
		/*		for (i = 0; i < BlocksArr.size(); i++) {
					if (checks[i] == true)
					{
						selectedBlock = BlocksArr[i];
					}
				}
		*/
		EstadoActual = Estado::Loading;
		EventoActual = Evento::StartExtraction;
		blocksEvento = true;
	}

	if (ImGui::Button("Volver al menu prinicpal"))
	{
		EstadoActual = Estado::MainMenu;
		EventoActual = Evento::gotoMainMenu;
		blocksEvento = true;
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();
	return blocksEvento;
}

bool Graphic::print_Error(void)
{
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	bool eventoError = false;

	ImGui::SetNextWindowPos(ImVec2(400, 270));
	ImGui::SetNextWindowSize(ImVec2(300, 70));

	ImGui::Begin("No existe el camino con blockes", 0, window_flags);

	if (ImGui::Button("Quit"))
	{
		EventoActual = Evento::Close;
		eventoError = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Volver al menu prinicpal"))
	{
		EstadoActual = Estado::MainMenu;
		EventoActual = Evento::gotoMainMenu;
		eventoError = true;
	}

	ImGui::End();

	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	al_flip_display();

	return eventoError;
}

void Graphic::flushVariables() {
	InputState = false;
	Error = false;
	close = false;
	path = "";
	directoryName = "";
}

void Graphic::print_Loading(void)
{
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(200, 10));
	ImGui::SetNextWindowSize(ImVec2(600, 100));

	ImGui::Begin("LOADING", 0, window_flags);
	ImGui::Text("Buscando informacion...");

	ImGui::End();

	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	al_flip_display();

}

bool Graphic::print_Done(void)
{
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(250, 100));
	ImGui::SetNextWindowSize(ImVec2(350, 300));
	bool eventoDone = false;


	ImGui::Begin("BlockInfo:");
	if (sizeof(selectedBlock) != 0)
	{


	}
	else
	{
		ImGui::Text("Ninguna Bloque se ha seleccionado");
	}
	if (ImGui::Button(" Quit "))
	{
		EventoActual = Evento::Close;
		eventoDone = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Volver al menu prinicpal"))
	{
		EstadoActual = Estado::MainMenu;
		EventoActual = Evento::gotoMainMenu;
		eventoDone = true;
	}

	ImGui::End();

	ImGui::Render();

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	//Allegro:
	if (sizeof(selectedBlock) != 0)
	{
		//PRINT MERKLE TREE
	}


	al_flip_display();

	return eventoDone;
}

/* FUNCIONES PARA USER */
void Graphic::processingRequest() // Le comunica a la gui que se llamo al algoritmo
{
	WorkInProgress = true;
}

bool Graphic::inputReady() // Te dice si el user ya seleccion� las imagenes deseadas
{
	return InputState;
}

void Graphic::error() // Le comunica a la gui que se realiz� la operaci�n exitosamente
{
	Result = false;
	WorkInProgress = false;
	EstadoActual = Estado::Error;
}

bool Graphic::AllegroInit()
{
	if (al_init())
	{
		if (al_init_image_addon())
		{
			if (al_install_mouse())
			{
				if (al_install_keyboard())
				{
					if (al_init_primitives_addon())
					{
						//Backgrounds[0] = al_load_bitmap("twitterbac.png");
						return true;
					}
					else
					{
						printf("ERROR al_init_primitives_addon");
						al_uninstall_keyboard();
						al_shutdown_image_addon();
						al_uninstall_mouse();
					}
				}
				else
				{
					printf("ERROR al_instal_keyboard\n");
					al_shutdown_image_addon();
					al_uninstall_mouse();
				}
			}
			else
			{
				printf("ERROR al_install_mouse\n");
				al_shutdown_image_addon();
			}
		}
		else
			printf("ERROR al_init_image_addon\n");
	}
	else
		printf("ERROR al_init\n");
	return false;
}

bool Graphic::ImguiInit(void)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	display = al_create_display(SIZE_SCREEN_X, SIZE_SCREEN_Y);
	if (display)
	{
		al_set_window_position(display, 500, 100); //posicion del menu

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		ImGui_ImplAllegro5_Init(display);
	}
	else
		return false;

	return true;
}

void Graphic::success() // Le comunica a la gui que se realiz� la operaci�n exitosamente
{
	Result = true;
	WorkInProgress = false;
	InputState = false;

	EstadoActual = Estado::InfoReady;
}