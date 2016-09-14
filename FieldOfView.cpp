
//
//  FieldOfView.cpp
//  FieldOfView
//
//  Created by James Vaughan Craster on 15/06/2016.
//  Copyright (c) 2016 James Vaughan Craster. All rights reserved.
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
/*#include "ResourcePath.hpp"*/

float pi = 3.1415926;

float calculateLambda(sf::Vector2f lightCenter, sf::Vector2f secondLightVertex, sf::Vector2f firstVertex, sf::Vector2f secondVertex){
    
    float lambda = ((lightCenter.y - firstVertex.y)*(secondVertex.x - firstVertex.x) - (lightCenter.x - firstVertex.x)*(secondVertex.y - firstVertex.y))/((secondLightVertex.x - lightCenter.x)*(secondVertex.y - firstVertex.y) - (secondLightVertex.y - lightCenter.y)*(secondVertex.x - firstVertex.x));
    
    return lambda;
                                                                                                                                                          
}

float calculateMew(sf::Vector2f lightCenter, sf::Vector2f secondLightVertex, sf::Vector2f firstVertex, sf::Vector2f secondVertex, float lambda){
    float mew;
    
    if(std::abs(secondVertex.x - firstVertex.x) < 0.01){
        mew = (lightCenter.y + lambda *(secondLightVertex.y - lightCenter.y) - firstVertex.y)/(secondVertex.y - firstVertex.y);
        
    }else{
         mew = (lightCenter.x + lambda*(secondLightVertex.x - lightCenter.x) - firstVertex.x)/(secondVertex.x - firstVertex.x);
        
        
    }
    return  mew;
    
}

sf::Vector2f calculateIntersect(sf::Vector2f lightCenter, sf::Vector2f secondLightVertex, float lambda){
    sf::Vector2f intersect;
    intersect.x = lightCenter.x + lambda * (secondLightVertex.x - lightCenter.x);
    intersect.y = lightCenter.y + lambda * (secondLightVertex.y - lightCenter.y);
    
    return intersect;
}

void drawBeam(sf::Vector2f lightCenter, float angle, float radius, std::vector<sf::VertexArray*>& edgeArray, sf::RenderWindow * window){
    float lambda;
    float mew;
    float storedLambda;
    
    sf::VertexArray triangleFan(sf::TrianglesFan);
    float cosVal;
    float sinVal;
    
    
    triangleFan.resize(1);
    triangleFan[0].position = lightCenter;
    triangleFan[0].color = sf::Color(255,0,255,40);
    
    
    for(float a = (angle - 3.14/5); a < (angle + 3.14/5); a += 0.0003)
    {
        cosVal = cos(a);
        sinVal = sin(a);
        storedLambda = 1;
        
        
        
        for(int i = 0; i < edgeArray.size(); i++)
        {
            lambda = calculateLambda(lightCenter, sf::Vector2f(lightCenter.x + radius * cosVal, lightCenter.y + radius * sinVal), (*edgeArray[i])[0].position, (*edgeArray[i])[1].position);
            if(lambda > 0 && lambda < storedLambda)
            {
                mew = calculateMew(lightCenter,sf::Vector2f(lightCenter.x + radius * cosVal, lightCenter.y + radius * sinVal) , (*edgeArray[i])[0].position, (*edgeArray[i])[1].position, lambda);
                
                if(mew > 0 && mew < 1)
                {
                    storedLambda = lambda;
                }
            }
        
         }
        triangleFan.resize(triangleFan.getVertexCount()+1);
        triangleFan[triangleFan.getVertexCount()-1].color = sf::Color(255,0,255,40);
        
        if(storedLambda == 1){
           
            triangleFan[triangleFan.getVertexCount()-1].position = sf::Vector2f(lightCenter.x + radius * cosVal, lightCenter.y + radius * sinVal);
             
        }else{
        
            triangleFan[triangleFan.getVertexCount()-1].position = calculateIntersect(lightCenter, sf::Vector2f(lightCenter.x + radius * cosVal, lightCenter.y + radius * sinVal), storedLambda);
        }
    
    
    }

    
    window->draw(triangleFan);
}
bool lightCollision(float lightAngle, sf::Vector2f occluderPos, sf::Vector2f lightCenter, float width, float radius,
                    std::vector<sf::VertexArray*> &edgeVector){
    float occluderAngle = atan2f(occluderPos.y - lightCenter.y, occluderPos.x - lightCenter.x);
    
    float tempangle = occluderAngle;
    float alpha = lightAngle;
    float magnitude = pow(pow(occluderPos.x - lightCenter.x,2) + pow(occluderPos.y - lightCenter.y, 2),0.5);
    float lambda;
    float mew;
    
    bool occluderInLight = 0;
    
    if(magnitude > radius){
        return occluderInLight;
    }
    
    
    if(tempangle < 0){
        tempangle *= -1;
        tempangle = (2 * pi) - tempangle;
    }
    
    
    if(tempangle < alpha + width &&
       tempangle > alpha - (width)){
        
        
            occluderInLight = 1;
        
        
    }else if(alpha + width >= 2 * pi){
        if(tempangle >= 0 && tempangle <= (alpha + width - 2 * pi)){
            
            occluderInLight = 1;
            
            
            
        }else if(tempangle <= 2* pi && tempangle > alpha - width)
        {
            
            occluderInLight = 1;
            
            
        }
        
    }else if(alpha - width <= 0){
        if(tempangle >= alpha - width + 2*pi && tempangle <= 2*pi)
        {
            
            occluderInLight = 1;
            
            
        }else if(tempangle <= alpha + width){
            
            occluderInLight = 1;
            
            
            
        }
        
    }
    if(occluderInLight == 0){
        return 0;
    }else{
        
        for(int i = 0; i < edgeVector.size(); i++)
        {
            lambda = calculateLambda(lightCenter, occluderPos, (*edgeVector[i])[0].position, (*edgeVector[i])[1].position);
            if(lambda > 0 && lambda < 1)
            {
                mew = calculateMew(lightCenter, occluderPos, (*edgeVector[i])[0].position, (*edgeVector[i])[1].position, lambda);
                
                if(mew > 0 && mew < 1)
                {
                    return 0;
                }
            }
            
        }
        return 1;
    }
    
    
}



class OccluderShape{
public:
    sf::ConvexShape body;
    std::vector<sf::VertexArray*> edgeVector;
    
    OccluderShape(int size){
        body.setPointCount(size);
    }
    
    void update(){
        edgeVector.clear();
        edgeVector.shrink_to_fit();
        
        
        
        for (int i = 0; i < body.getPointCount(); i++) {
            
            if( i == body.getPointCount()-1)
            {
                edgeVector.push_back(new sf::VertexArray);
                edgeVector[edgeVector.size()-1]->resize(2);
                
                (*edgeVector[edgeVector.size()-1])[0].position = body.getPoint(i);
                
                (*edgeVector[edgeVector.size()-1])[0].color = sf::Color(255,255,255,255);
                (*edgeVector[edgeVector.size()-1])[1].position = body.getPoint(0);
                (*edgeVector[edgeVector.size()-1])[1].color = sf::Color(255,255,255,255);
                (*edgeVector[edgeVector.size()-1]).setPrimitiveType(sf::LinesStrip);
                
           
            
            
            }else{
            edgeVector.push_back(new sf::VertexArray);
            edgeVector[edgeVector.size()-1]->resize(2);
            
            (*edgeVector[edgeVector.size()-1])[0].position = body.getPoint(i);
            (*edgeVector[edgeVector.size()-1])[0].color = sf::Color(255,255,255,255);
            (*edgeVector[edgeVector.size()-1])[1].position = body.getPoint(i+1);
            (*edgeVector[edgeVector.size()-1])[1].color = sf::Color(255,255,255,255);
            (*edgeVector[edgeVector.size()-1]).setPrimitiveType(sf::LinesStrip);
            }
        }
    }
};


int main(int, char const**)
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1400, 900), "SFML window");
    window.setFramerateLimit(30);
    
    float angle;
    
    sf::Vector2f lightCenter = sf::Vector2f(400,300);
    
    float radius = 600;
    sf::RectangleShape rect;
    rect.setOutlineColor(sf::Color(255,0,255,255));
    rect.setOutlineThickness(1);
    
    std::vector<sf::VertexArray*> edgeVector;
   
    sf::VertexArray edge(sf::LinesStrip);
    edge.resize(2);
    edge[0].position = sf::Vector2f(550,400);
    edge[1].position = sf::Vector2f(500, 200);
    edgeVector.push_back(&edge);
    
    sf::VertexArray edge2(sf::LinesStrip);
    edge2.resize(2);
    edge2[0].position = sf::Vector2f(650,200);
    edge2[1].position = sf::Vector2f(500, 200);
    edgeVector.push_back(&edge2);
    
    std::vector<sf::VertexArray*> tempEdgeVector;
    
    sf::ConvexShape triangle(3);
    float lambda;
    float mew;
    
    sf::RectangleShape player(sf::Vector2f(50,50));
    player.setFillColor(sf::Color::Green);
    player.setPosition(0,0);
    
    sf::Vector2f playerCenter = sf::Vector2f(player.getPosition().x + player.getSize().x/2, player.getPosition().y + player.getSize().y/2);
    
    sf::Transform rotation;
    rotation.rotate(1, (*edgeVector[1])[0].position);
  
    
    OccluderShape shape(3);
    shape.body.setPoint(0, sf::Vector2f(300,300));
    shape.body.setPoint(1, sf::Vector2f(400,300));
    shape.body.setPoint(2, sf::Vector2f(300,400));
    shape.update();
    
    
    for(int a = 0; a < shape.edgeVector.size(); a++){
        edgeVector.push_back(shape.edgeVector[a]);
        
    }

    
    
    
    
    // Start the game loop
    while (window.isOpen())
    {
        
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }
        angle = atan2f(sf::Mouse::getPosition(window).y - lightCenter.y, sf::Mouse::getPosition(window).x - lightCenter.x);
        
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            player.move(1, 0);
        
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            player.move(-1, 0);
        
        
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            player.move(0, -1);
        
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            player.move(0, 1);
        
        }
        
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {
            
            edgeVector.resize(edgeVector.size()+1);
            
            edgeVector[edgeVector.size()-1] = new sf::VertexArray(sf::LinesStrip);
            
            (*edgeVector[edgeVector.size()-1]).resize(2);
            
            (*edgeVector[edgeVector.size()-1])[0].position = sf::Vector2f(sf::Mouse::getPosition(window));
            (*edgeVector[edgeVector.size()-1])[0].color = sf::Color(255,255,255,255);
            
            (*edgeVector[edgeVector.size()-1])[1].position = sf::Vector2f(300,600);
            (*edgeVector[edgeVector.size()-1])[1].color = sf::Color(255,255,255,255);
            
            
            
            
            
            
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
            lightCenter.y -= 1;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
            lightCenter.y += 1;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
            lightCenter.x -= 1;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
            lightCenter.x += 1;
        }
        
        playerCenter = sf::Vector2f(player.getPosition().x + player.getSize().x/2, player.getPosition().y + player.getSize().y/2);
        
        triangle.setPoint(0, lightCenter);
        
        triangle.setPoint(1, sf::Vector2f(lightCenter.x + radius/cos(3.14/5) * cos(angle - 3.14/5), lightCenter.y + radius/cos(3.14/5) * sin(angle - 3.14/5)));
        
        triangle.setPoint(2, sf::Vector2f(lightCenter.x + radius/cos(3.14/5) * cos(angle + 3.14/5), lightCenter.y + radius/cos(3.14/5) * sin(angle + 3.14/5)));
        
        tempEdgeVector.clear();
        window.clear();
      if(angle < 0){
            angle *= -1;
            angle = (2 * pi) - angle;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::C)){
       (*edgeVector[1])[1].position =  rotation.transformPoint((*edgeVector[1])[1].position);
        }
        if(lightCollision(angle, playerCenter, lightCenter, pi/5, radius, edgeVector) == 0){
            player.setFillColor(sf::Color::Green);
        }else{
            player.setFillColor(sf::Color::Red);
        }
        
      
        drawBeam(lightCenter, angle,  radius, edgeVector, &window);
        
        for(int i = 0; i < edgeVector.size(); i++){
            window.draw(*edgeVector[i]);
            
        }
    
        window.draw(rect);
        window.draw(player);
        window.display();
    }

    return EXIT_SUCCESS;
}
