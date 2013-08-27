#include "InputState.h"

void InputState::ResetOneFrameKeys()
{
	jumpHeld = false;
	useHeld = false;
	healthHeld = false;
	resistHeld = false;
	talkHeld = false;
	exitHeld = false;
	mouseChange.Reset();
}

void InputState::ResetAll()
{
	forwardHeld = false;
	backwardHeld = false;
	leftHeld = false;
	rightHeld = false;
	speedHeld = false;
	primaryFireHeld = false;
	secondaryFireHeld = false;
	tertiaryFireHeld = false;
	ResetOneFrameKeys();

}

void InputState::Update( sf::RenderWindow &window )
{
	sf::Event event;

	while ( window.pollEvent( event ) )
    {
        if ( event.type == sf::Event::KeyReleased )
        {

        }
        else if ( event.type == sf::Event::KeyPressed )
        {

        }
        else if ( event.type == sf::Event::Closed )
        {
            exitHeld = true;
        }
    }

	//check button releases
	//set any released keys to false

	//check button presses
	//set any pressed keys to true
}

bool        InputState::GetForwardHeld() const 		    { return forwardHeld; }
bool        InputState::GetBackwardHeld() const 		{ return backwardHeld; }
bool        InputState::GetLeftHeld() const 			{ return leftHeld; }
bool        InputState::GetRightHeld() const 			{ return rightHeld; }
bool        InputState::GetJumpHeld() const 			{ return jumpHeld; }
bool        InputState::GetSpeedHeld() const 			{ return speedHeld; }
bool        InputState::GetUseHeld() const 			    { return useHeld; }
bool        InputState::GetHealthHeld() const 			{ return healthHeld; }
bool        InputState::GetResistHeld() const 			{ return resistHeld; }
bool        InputState::GetPrimaryFireHeld() const 	    { return primaryFireHeld; }
bool        InputState::GetSecondaryFireHeld() const 	{ return secondaryFireHeld; }
bool        InputState::GetTertiaryFireHeld() const 	{ return tertiaryFireHeld; }
bool        InputState::GetTalkHeld() const 			{ return talkHeld; }
bool        InputState::GetExitHeld() const 			{ return exitHeld; }
frmr::Vec2f InputState::GetMouseChange() const 		    { return mouseChange; }

InputState::InputState()
{
	ResetAll();
}

InputState::~InputState()
{
}
