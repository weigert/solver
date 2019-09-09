#include "solver.cpp"
/*
================================================================================
                            PDE Solving Helper Class
================================================================================
*/
template<typename Model>
class Solver{
public:
  //Setup the Simulation
  std::string name;
  std::vector<CArray> fields;
  //const char* field_names[] = {"Volcanism", "Plates", "Height"};

  glm::vec2 dim;
  double timeStep = 1.0;
  int steps = 0;              //Remaining Steps
  bool updateFields = true;  //If the fields have been updated

  //Grid Set Manipulators
  void addField(float a[]);
  void addField(CArray a);
  void appendFields(std::vector<CArray> a);

  //Current Integrator Handle
  std::vector<CArray>(Model::*_caller)(std::vector<CArray>&);

  //Master Integrator (this is called every tick to integrate a single step)
  bool integrate(Model &model, std::vector<CArray> (Solver::*_inte)( Model &model, std::vector<CArray>(Model::*_call)(std::vector<CArray>&_fields)));

  //Step Integration Methods
  std::vector<CArray> DIRECT(Model &model, std::vector<CArray> (Model::*_call)( std::vector<CArray> &_fields ) );
  std::vector<CArray> EE(Model &model, std::vector<CArray> (Model::*_call)( std::vector<CArray> &_fields ) );
};

/*
================================================================================
                      Helpers (Field Adders / Handling)
================================================================================
*/
template<typename Model>
void Solver<Model>::addField(CArray a){
  //Make sure to set the modes!
  solve::modes = dim;
  fields.push_back(a);
}

template<typename Model>
void Solver<Model>::addField(float a[]){
  //Make sure to set the modes!
  solve::modes = dim;
  fields.push_back(solve::fromArray(a));
}

template<typename Model>
void Solver<Model>::appendFields(std::vector<CArray> a){
  solve::modes = dim;
  for(unsigned int i = 0; i < a.size(); i++){
    fields.push_back(a[i]);
  }
}


/*
================================================================================
                              Solver Functions
================================================================================
*/

//Master Integrator
template<typename Model>
bool Solver<Model>::integrate(Model &model, std::vector<CArray> (Solver::*_inte)( Model &model, std::vector<CArray> (Model::*_call)(std::vector<CArray> &_fields) )){
  //Set the modes
  solve::modes = dim;

  //If we have any steps left, perform an integration step
  if(steps != 0){
    //Get the Deltas
    std::vector<CArray> deltas = (*this.*_inte)(model, _caller);

    //Add the Deltas
    for(unsigned int i = 0; i < fields.size(); i++){
      //Add the Terms to the fields
      fields[i] += deltas[i];
    }

    //Subtract a step
    steps--;
  }

  //Fields have been update
  updateFields = true;

  //Really, everything should be converted into the fourier space here...
  return true;
}

/*
================================================================================
                              Time Integrators
================================================================================
*/

//Explicit Euler Integrator
template<typename Model>
std::vector<CArray> Solver<Model>::EE(Model &model, std::vector<CArray> (Model::*_call)( std::vector<CArray> &_fields ) ){
  //Get the Lambdas
  std::vector<CArray> lambdas = (model.*_call)(fields);
  std::for_each(lambdas.begin(), lambdas.end(), [this](CArray &l){ l = (complex)timeStep*l;});
  return lambdas;
}

//Explicit Euler Integrator
template<typename Model>
std::vector<CArray> Solver<Model>::DIRECT(Model &model, std::vector<CArray> (Model::*_call)( std::vector<CArray> &_fields ) ){
  //Get the Lambdas
  std::vector<CArray> lambdas = (model.*_call)(fields);
  return lambdas;
}