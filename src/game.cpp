#include "game.h"

Game::Game() : 
	m_interrupted(true),
	m_physics_wrapper(nullptr),
	m_thread(nullptr)
{
	initialize();
}

Game::~Game(){
	m_thread.release();
}

void Game::initialize(){
	m_physics_wrapper.reset(new PhysicsWrapper());
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
	m_thread.release();
	m_thread.reset(new std::thread(&Game::loop, this));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool Game::isRunning()const{
	return !m_interrupted;
}

std::shared_ptr<PhysicsWrapper> Game::getPhysicsWrapper(){
	return m_physics_wrapper;
}