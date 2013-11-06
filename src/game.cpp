#include "game.h"

Game::Game() : 
	m_interrupted(true),
	m_physics_wrapper(nullptr),
	m_thread(nullptr)
{
	initialize();
}

Game::~Game(){
	if (m_thread){
		delete m_thread;
		m_thread = nullptr;
	}
	if (m_physics_wrapper){
		delete m_physics_wrapper;
		m_physics_wrapper = nullptr;
	}
}

void Game::initialize(){
	m_physics_wrapper = new PhysicsWrapper();
}

void Game::fatalError(string error_message){
	std::cerr << "Game Error occured:" << std::endl;
	std::cerr << error_message << std::endl;
	std::cerr << "Press Enter to close the Application" << std::endl;
	string temp;
	std::getline(std::cin, temp);
	exit(1);
}

void Game::start(){
	if (m_thread){
		delete m_thread;
		m_thread = nullptr;
	}
	m_thread = new std::thread(&Game::loop, this);
}

void Game::interrupt(){
	m_interrupted = true;
}

void Game::end(){
	if (isRunning()){
		interrupt();
		m_thread->join();
	}
}

void Game::loop(){
	m_interrupted = false;
	while (isRunning())
	{
		m_physics_wrapper->step(100);
		_sleep(100);
	}
}

bool Game::isRunning(){
	return !m_interrupted;
}