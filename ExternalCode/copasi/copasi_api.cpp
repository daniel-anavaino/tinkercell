/**
* This is an example on how to build models with the COPASI backend API.
*/
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <QString>
#include <QStringList>
#include <QRegExp>

#define COPASI_MAIN
#include "copasi_api.h"
#include "copasi/copasi.h"
#include "copasi/report/CCopasiRootContainer.h"
#include "copasi/CopasiDataModel/CCopasiDataModel.h"
#include "copasi/model/CModel.h"
#include "copasi/model/CCompartment.h"
#include "copasi/model/CMetab.h"
#include "copasi/model/CReaction.h"
#include "copasi/model/CChemEq.h"
#include "copasi/model/CModelValue.h"
#include "copasi/function/CFunctionDB.h"
#include "copasi/function/CFunction.h"
#include "copasi/function/CEvaluationTree.h"
#include "copasi/report/CCopasiRootContainer.h"
#include "copasi/report/CReport.h"
#include "copasi/report/CReportDefinition.h"
#include "copasi/report/CReportDefinitionVector.h"
#include "copasi/trajectory/CTrajectoryTask.h"
#include "copasi/trajectory/CTrajectoryMethod.h"
#include "copasi/trajectory/CTrajectoryProblem.h"
#include "copasi/scan/CScanTask.h"
#include "copasi/scan/CScanMethod.h"
#include "copasi/scan/CScanProblem.h"
#include "copasi/trajectory/CTimeSeries.h"

extern "C"
{
	#define LIB_EXPORTS 1 //for antimony
	#include "src/antimony_api.h"
}

void copasi_init()
{
	CCopasiRootContainer::init(0, NULL);
}

void copasi_end()
{
	CCopasiRootContainer::destroy();
}

copasi_model createCopasiModel(const char * name)
{
	CCopasiDataModel* pDataModel = CCopasiRootContainer::addDatamodel();
	CModel* pModel = pDataModel->getModel();
	copasi_model m = { (void*)(pModel) , (void*)(pDataModel) };
	pModel->setSBMLId( std::string(name) );
	pModel->setObjectName( std::string(name) );
	//pModel->setTimeUnit(CModel::s);
	//pModel->setVolumeUnit(CModel::microl);
	//pModel->setQuantityUnit(CModel::nMol);
	pModel->setTimeUnit(CModel::dimensionlessTime);
	pModel->setVolumeUnit(CModel::dimensionlessVolume);
	pModel->setQuantityUnit(CModel::dimensionlessQuantity);
	return m;
}

copasi_species createSpecies(copasi_compartment compartment, const char* name, double iv)
{
	CModel* pModel = (CModel*)(compartment.CopasiModelPtr);
	CCompartment* pCompartment = (CCompartment*)(compartment.CopasiCompartmentPtr);
	CMetab* pSpecies = pModel->createMetabolite(name, pCompartment->getObjectName(), iv, CMetab::REACTIONS);
	copasi_species s = { (void*)(pSpecies), (void*)(pModel) };
	return s;
}

copasi_compartment createCompartment(copasi_model model, const char* name, double volume)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CCompartment* pCompartment = pModel->createCompartment(name, volume);
	copasi_compartment c = { (void*)(pCompartment), (void*)(pModel) };
	return c;
}

void setConcentration(copasi_species species, double initialValue)
{
	CMetab* pSpecies = (CMetab*)(species.CopasiSpeciesPtr);
	pSpecies->setConcentration(initialValue);
}

copasi_parameter setGlobalParameter(copasi_model model, const char * name, double value)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	int i;
	std::string s(name);
	CCopasiVectorN< CModelValue > & params = pModel->getModelValues();
	CModelValue * pValue = 0;
	copasi_parameter p = { (void*)(pValue), (void*)(pModel) };
	
	for (i=0; i < params.size(); ++i)
	{
		if (params[i] && params[i]->getObjectName().compare( s ) == 0)
		{
			params[i]->setValue(value);
			pValue = params[i];
			break;
		}
	}
	
	//parameter not found, so create it
	if (!pValue)
		pValue = pModel->createModelValue(s,value);
	
	p.CopasiParameterPtr = (void*)pValue;
	return p;
}

void setBoundarySpecies(copasi_species species, int isBoundary)
{
	CMetab* pSpecies = (CMetab*)(species.CopasiSpeciesPtr);
	if (isBoundary)
		pSpecies->setStatus(CModelEntity::FIXED);
	else
		pSpecies->setStatus(CModelEntity::REACTIONS);
}

void setAssignmentRule(copasi_species species, const char * formula)
{
	CMetab* pSpecies = (CMetab*)(species.CopasiSpeciesPtr);
	if (formula)
	{
		pSpecies->setStatus(CModelEntity::ASSIGNMENT);
		pSpecies->setExpression(std::string(formula));
	}
	else
		pSpecies->setStatus(CModelEntity::REACTIONS);
}

copasi_parameter createVariable(copasi_model model, const char * name, const char * formula)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CModelValue* pModelValue = pModel->createModelValue(std::string(name), 0.0);
	copasi_parameter p = { (void*)(pModelValue), (void*)(pModel) };
	pModelValue->setStatus(CModelValue::ASSIGNMENT);
	int i,j,k;

	CCopasiVectorNS < CCompartment > & compartments = pModel->getCompartments();
	CCopasiVector< CMetab > & species = pModel->getMetabolites();
	CCopasiVectorN< CModelValue > & params = pModel->getModelValues();
	
	CFunction pFunction;
	QString qFormula(formula);
	bool ok;

	if (pFunction.setInfix(std::string(formula)))
	{
		CFunctionParameters& variables = pFunction.getVariables();
		CFunctionParameter* pParam;

		for (i=0; i < variables.size(); ++i)
		{
			ok = false;
			pParam = variables[i];

			QRegExp regexp(QString("(^[A-Za-z0-9_])") + QString(pParam->getObjectName().c_str()) + QString("(^[A-Za-z0-9_])"));
			
			for (j=0; j < compartments.size(); ++j)
					if (compartments[j] && compartments[j]->getObjectName().compare(pParam->getObjectName())==0)
					{
						QString s("<");
						s += QString(compartments[j]->getCN().c_str());
						s += QString(">");
						qFormula.replace(regexp, QString("\\1")+ s+QString("\\2") );
						ok = true;
						break;
					}
			if (ok) continue;
			for (j=0; j < species.size(); ++j)
				if (species[j] && species[j]->getObjectName().compare(pParam->getObjectName())==0)
				{
					QString s("<");
					s += QString(species[j]->getCN().c_str());
					s += QString(">");
					qFormula.replace(regexp, QString("\\1")+ s+QString("\\2") );
					ok = true;
					break;
				}
			if (ok) continue;
			for (j=0; j < params.size(); ++j)
				if (params[j] && params[j]->getObjectName().compare(pParam->getObjectName())==0)
				{
					QString s("<");
					s += QString(params[j]->getCN().c_str());
					s += QString(">");
					qFormula.replace(regexp, QString("\\1")+ s+QString("\\2") );
					ok = true;
					break;
				}
		}
	}

	std::string sFormula( qFormula.toAscii().data() );
	std::cout << sFormula << std::endl;
	pModelValue->setInitialExpression(sFormula);
	pModelValue->setExpression(sFormula);
	return p;
}

copasi_parameter createVariable_v2(copasi_model model, const char * name, const char * formula)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CModelValue* pModelValue = pModel->createModelValue(std::string(name), 0.0);
	copasi_parameter p = { (void*)(pModelValue), (void*)(pModel) };
	pModelValue->setStatus(CModelValue::ASSIGNMENT);
	pModelValue->setInitialExpression(formula);
	pModelValue->setExpression(formula);
	return p;
}

copasi_reaction createReaction(copasi_model model, const char* name)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CReaction* pReaction = pModel->createReaction(name);
	copasi_reaction r = { (void*)(pReaction), (void*)(pModel) };
	return r;
}

void addReactant(copasi_reaction reaction, copasi_species species, double stoichiometry)
{
	CReaction* pReaction = (CReaction*)(reaction.CopasiReactionPtr);
	CMetab* pSpecies = (CMetab*)(species.CopasiSpeciesPtr);
	CChemEq* pChemEq = &pReaction->getChemEq();
	pChemEq->addMetabolite(pSpecies->getKey(), stoichiometry, CChemEq::SUBSTRATE);
}

void addProduct(copasi_reaction reaction, copasi_species species, double stoichiometry)
{
	CReaction* pReaction = (CReaction*)(reaction.CopasiReactionPtr);
	CMetab* pSpecies = (CMetab*)(species.CopasiSpeciesPtr);
	CChemEq* pChemEq = &pReaction->getChemEq();
	pChemEq->addMetabolite(pSpecies->getKey(), stoichiometry, CChemEq::PRODUCT);
}

int setReactionRate(copasi_reaction reaction, const char * formula)
{
	int i,j,k;
	CReaction* pReaction = (CReaction*)(reaction.CopasiReactionPtr);
	CModel* pModel = (CModel*)(reaction.CopasiModelPtr);
	CFunctionDB* pFunDB = CCopasiRootContainer::getFunctionList();
	
	CCopasiVectorNS < CCompartment > & compartments = pModel->getCompartments();
	CCopasiVector< CMetab > & species = pModel->getMetabolites();
	CCopasiVectorN< CModelValue > & params = pModel->getModelValues();
	
	if (pFunDB)
	{
		std::string rateLawName(pReaction->getObjectName() + std::string("_rate_law")); //existing rate law
		
		CFunction * pFunction = dynamic_cast<CFunction*>(pFunDB->findFunction(rateLawName));
		if (pFunction)
			return (int)(pReaction->setFunction(pFunction)) - 1;

		CKinFunction* pKinFunction = new CKinFunction(rateLawName);
		pFunDB->add(pKinFunction, true);
		pFunction = pKinFunction;//dynamic_cast<CFunction*>(pFunDB->findFunction(rateLawName));
		
		if (!pFunction)
			return -1;
		
		pFunction->setReversible(TriFalse);
		
		bool ok;
		int retval = -1;

		if (pFunction->setInfix(std::string(formula)))
		{
			retval = (int)(pReaction->setFunction(pFunction)) - 1;
			CFunctionParameters& variables = pFunction->getVariables();
			CFunctionParameter* pParam;

			for (i=0; i < variables.size(); ++i)
			{
				ok = false;
				pParam = variables[i];
				if (pParam->getObjectName().compare(std::string("time"))==0 ||
				     pParam->getObjectName().compare(std::string("Time"))==0 ||
				     pParam->getObjectName().compare(std::string("TIME"))==0)
				{
					pParam->setUsage(CFunctionParameter::TIME);
					ok = true;
				}
				if (ok) continue;
				for (j=0; j < compartments.size(); ++j)
					if (compartments[j] && compartments[j]->getObjectName().compare(pParam->getObjectName())==0)
					{
						pParam->setUsage(CFunctionParameter::VOLUME);
						pReaction->setParameterMapping(pParam->getObjectName(), compartments[j]->getKey());
						ok = true;
						break;
					}
				if (ok) continue;
				for (j=0; j < species.size(); ++j)
					if (species[j] && species[j]->getObjectName().compare(pParam->getObjectName())==0)
					{
						pParam->setUsage(CFunctionParameter::MODIFIER);
						const CCopasiVector < CChemEqElement > & substrates = pReaction->getChemEq().getSubstrates();
						for (k =0; k < substrates.size(); ++k)
							if (substrates[k]->getMetabolite() == species[j])
							{
								pParam->setUsage(CFunctionParameter::SUBSTRATE);
								break;
							}
						pReaction->setParameterMapping(pParam->getObjectName(), species[j]->getKey());
						ok = true;
						break;
					}
				if (ok) continue;
				for (j=0; j < params.size(); ++j)
					if (params[j] && params[j]->getObjectName().compare(pParam->getObjectName())==0)
					{
						pParam->setUsage(CFunctionParameter::PARAMETER);
						pReaction->setParameterMapping(pParam->getObjectName(), params[j]->getKey());
						ok = true;
						break;
					}
			}
			
			pFunction->compile();
			
			return retval;
		}
	}

	return -1;
	//const CFunction * function = pReaction->getFunction();
	//CChemEq* pChemEq = &pReaction->getChemEq();
}

int setReactionRate_v2(copasi_reaction reaction, const char * formula, int sbo, char ** paramNames, copasi_parameter * paramMappings, char ** speciesNames, copasi_species * speciesMappings)
{
	int i,j,k;
	CReaction* pReaction = (CReaction*)(reaction.CopasiReactionPtr);
	CModel* pModel = (CModel*)(reaction.CopasiModelPtr);
	CFunctionDB* pFunDB = CCopasiRootContainer::getFunctionList();
	CMetab * pSpecies = 0;
	CModelValue * pValue = 0;
	CCopasiVectorNS < CCompartment > & compartments = pModel->getCompartments();
	
	if (pFunDB)
	{
		CFunction * pFunction = dynamic_cast<CFunction*>(pFunDB->findFunction(std::string(formula))); //SBO rate law
		std::string rateLawName(pReaction->getObjectName() + std::string("_rate_law")); //existing rate law
		
		if (!pFunction)
			pFunction = dynamic_cast<CFunction*>(pFunDB->findFunction(rateLawName));
		
		if (!pFunction)
		{
			CKinFunction* pKinFunction = new CKinFunction(rateLawName);
			pFunDB->add(pKinFunction, true);
			pFunction = pKinFunction;
			pFunction->setReversible(TriFalse);
		}
		
		if (!pFunction)
			return -1;
		
		bool ok;
		int retval = -1;

		if (pFunction->setInfix(std::string(formula)))
		{
			retval = (int)(pReaction->setFunction(pFunction)) - 1;
			CFunctionParameters& variables = pFunction->getVariables();
			CFunctionParameter* pParam;

			//TIME
			unsigned C_INT32 index = pFunction->getVariableIndex("time");
			if (index != C_INVALID_INDEX)
			{
				pParam = variables[index];
				pParam->setUsage(CFunctionParameter::TIME);
			}
			index = pFunction->getVariableIndex("TIME");
			if (index != C_INVALID_INDEX)
			{
				pParam = variables[index];
				pParam->setUsage(CFunctionParameter::TIME);
			}
			index = pFunction->getVariableIndex("Time");
			if (index != C_INVALID_INDEX)
			{
				pParam = variables[index];
				pParam->setUsage(CFunctionParameter::TIME);
			}
			
			//Compartments
			for (i=0; i < variables.size(); ++i)
			{
				for (j=0; j < compartments.size(); ++j)
					if (compartments[j] && compartments[j]->getObjectName().compare(pParam->getObjectName())==0)
					{
						pParam->setUsage(CFunctionParameter::VOLUME);
						pReaction->setParameterMapping(pParam->getObjectName(), compartments[j]->getKey());
						break;
					}
			}
			
			//given parameters
			for (j=0; paramNames != 0 && paramMappings != 0 && paramNames[j] != 0; ++j)
			{
				index = pFunction->getVariableIndex(paramNames[j]);
				if (index != C_INVALID_INDEX)
				{
					pParam = variables[index];
					pParam->setUsage(CFunctionParameter::PARAMETER);
					pValue = (CModelValue*)(paramMappings[j].CopasiParameterPtr);
					pReaction->setParameterMapping(std::string(paramNames[j]), pValue->getKey());
					break;
				}
			}
			
			//given species
			for (j=0; speciesNames != 0 && speciesMappings != 0 && speciesNames[j] != 0; ++j)
			{
				index = pFunction->getVariableIndex(speciesNames[j]);
				if (index != C_INVALID_INDEX)
				{
					pParam = variables[index];
					pParam->setUsage(CFunctionParameter::MODIFIER);
					pSpecies = (CMetab*)(speciesMappings[j].CopasiSpeciesPtr);
					const CCopasiVector < CChemEqElement > & substrates = pReaction->getChemEq().getSubstrates();
					for (k =0; k < substrates.size(); ++k)
						if (substrates[k]->getMetabolite() == pSpecies)
						{
							pParam->setUsage(CFunctionParameter::SUBSTRATE);
							break;
						}
					pReaction->setParameterMapping(std::string(speciesNames[j]), pSpecies->getKey());
					break;
				}
			}
			
			pFunction->compile();
			
			return retval;
		}
	}

	return -1;
	//const CFunction * function = pReaction->getFunction();
	//CChemEq* pChemEq = &pReaction->getChemEq();
}

const char * getCopasiSpeciesID(copasi_species species, int * stringSize )
{
	std::string id = ((CMetab*)species.CopasiSpeciesPtr)->getCN();
	if (stringSize)
		(*stringSize) = id.length();
	
	char * cstr = new char [id.size()+1];
	strcpy (cstr, id.c_str());

	return cstr;
}

const char * getCopasiParameterID(copasi_parameter param, int * stringSize )
{
	std::string id = ((CModelValue*)param.CopasiParameterPtr)->getCN();
	if (stringSize)
		(*stringSize) = id.length();
	
	char * cstr = new char [id.size()+1];
	strcpy (cstr, id.c_str());

	return cstr;
}

void compileCopasiModel(copasi_model model)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CCopasiVectorNS < CCompartment > & compartments = pModel->getCompartments();
	CCopasiVector< CMetab > & species = pModel->getMetabolites();
	CCopasiVectorN< CModelValue > & params = pModel->getModelValues();
	const CCopasiObject* pObject = 0;
	std::set<const CCopasiObject*> changedObjects;
	
	for (int i=0; i < compartments.size(); ++i)
		if (compartments[i])
		{
			pObject = compartments[i]->getObject(CCopasiObjectName("Reference=InitialVolume"));
			if (pObject)
				changedObjects.insert(pObject);
		}

	for (int i=0; i < species.size(); ++i)
		if (species[i])
		{
			pObject = species[i]->getObject(CCopasiObjectName("Reference=InitialConcentration"));
			if (pObject)
				changedObjects.insert(pObject);
		}

	for (int i=0; i < params.size(); ++i)
		if (params[i])
		{
			pObject = params[i]->getObject(CCopasiObjectName("Reference=Value"));
			if (pObject)
				changedObjects.insert(pObject);
			
			pObject = params[i]->getObject(CCopasiObjectName("Reference=InitialValue"));
			if (pObject)
				changedObjects.insert(pObject);
		}

	// compile needs to be done before updating all initial values for
	// the model with the refresh sequence
	pModel->compileIfNecessary(NULL);
	
	// now that we are done building the model, we have to make sure all
	// initial values are updated according to their dependencies
	std::vector<Refresh*> refreshes = pModel->buildInitialRefreshSequence(changedObjects);
	
	std::vector<Refresh*>::iterator it2 = refreshes.begin(), endit2 = refreshes.end();
	
	while (it2 != endit2)
	{
		// call each refresh
		(**it2)();
		++it2;
	}
}

tc_matrix simulate(copasi_model model, double startTime, double endTime, int numSteps, CCopasiMethod::SubType method)
{
	CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CCopasiDataModel* pDataModel = (CCopasiDataModel*)(model.CopasiDataModelPtr);
	compileCopasiModel(model);
	
	// get the task list
	CCopasiVectorN< CCopasiTask > & TaskList = * pDataModel->getTaskList();
	// get the trajectory task object
	CTrajectoryTask* pTask = dynamic_cast<CTrajectoryTask*>(TaskList["Time-Course"]);
	// if there isn’t one
	if (pTask == NULL)
	{
		// create a new one
		pTask = new CTrajectoryTask();
		// remove any existing trajectory task just to be sure since in
		// theory only the cast might have failed above
		TaskList.remove("Time-Course");
		// add the new time course task to the task list
		TaskList.add(pTask, true);
	}
	
	if (startTime >= endTime)
		endTime += startTime;
	
	if (pTask && pTask->setMethodType(method))
	{
		//set the start and end time, number of steps, and save output in memory
		CTrajectoryProblem* pProblem=(CTrajectoryProblem*)pTask->getProblem();
		pProblem->setModel(pModel);
		pTask->setScheduled(true);
		pProblem->setStepNumber(numSteps);
		pProblem->setDuration(endTime-startTime);
		pDataModel->getModel()->setInitialTime(startTime);
		pProblem->setTimeSeriesRequested(true);
		try
		{
			pTask->initialize(CCopasiTask::OUTPUT_COMPLETE, pDataModel, NULL);
			pTask->process(true);
			pTask->restore();
		}
		catch(...)
		{
			std::cerr << "Error. Running the simulation failed." << std::endl;
			// check if there are additional error messages
			if (CCopasiMessage::size() > 0)
			{
				// print the messages in chronological order
				std::cerr << CCopasiMessage::getAllMessageText(true);
			}
			pTask = 0;
		}
	}
	
	if (pTask)
	{
		const CTimeSeries & timeSeries = pTask->getTimeSeries();
		int rows = timeSeries.getRecordedSteps(), cols = timeSeries.getNumVariables();
		int i,j;
	
		tc_matrix output = tc_createMatrix(rows, cols);
	
		for (j=0; j < cols; ++j)
			tc_setColumnName( output, j, timeSeries.getTitle(j).c_str()  );
	
		for (i=0; i < rows; ++i)
			for (j=0; j < cols; ++j)
				tc_setMatrixValue( output, i, j, timeSeries.getConcentrationData(i,j) );
	
		return output;
	}
	return tc_createMatrix(0,0);
}

tc_matrix simulateDeterministic(copasi_model model, double startTime, double endTime, int numSteps)
{
	return simulate(model,startTime,endTime,numSteps,CCopasiMethod::deterministic);
}

tc_matrix simulateTauLeap(copasi_model model, double startTime, double endTime, int numSteps)
{
	return simulate(model,startTime,endTime,numSteps,CCopasiMethod::tauLeap);
}

tc_matrix simulateStochastic(copasi_model model, double startTime, double endTime, int numSteps)
{
	return simulate(model,startTime,endTime,numSteps,CCopasiMethod::directMethod);
}

tc_matrix simulateHybrid(copasi_model model, double startTime, double endTime, int numSteps)
{
	return simulate(model,startTime,endTime,numSteps,CCopasiMethod::hybridLSODA);
}

copasi_model loadModelFile(const char * filename)
{
	loadFile(filename); //load Antimony or SBML (from antimony_api.h)
	char * s = getSBMLString("__main");  //Antimony -> SBML  (from antimony_api.h)
	CCopasiDataModel* pDataModel = CCopasiRootContainer::addDatamodel();
	pDataModel->importSBMLFromString(s); //SBML -> COPASI
	CModel* pModel = pDataModel->getModel();
	copasi_model m = { (void*)(pModel) , (void*)(pDataModel) };
	freeAll(); //free Antimony  (from antimony_api.h)
	return m;
}

tc_matrix getSteadyStates(copasi_model model, const char * parameter, double startvalue, double endvalue)
{
	/*CModel* pModel = (CModel*)(model.CopasiModelPtr);
	CCopasiDataModel* pDataModel = (CCopasiDataModel*)(model.CopasiDataModelPtr);
	compileCopasiModel(model);
	
	// get the task list
	CCopasiVectorN< CCopasiTask > & TaskList = * pDataModel->getTaskList();
	// get the trajectory task object
	CTrajectoryTask* pTask = dynamic_cast<CTrajectoryTask*>(TaskList["Time-Course"]);
	// if there isn’t one
	if (pTask == NULL)
	{
		// create a new one
		pTask = new CTrajectoryTask();
		// remove any existing trajectory task just to be sure since in
		// theory only the cast might have failed above
		TaskList.remove("Time-Course");
		// add the new time course task to the task list
		TaskList.add(pTask, true);
	}
	
	if (startTime >= endTime)
		endTime += startTime;
	
	if (pTask && pTask->setMethodType(method))
	{
		//set the start and end time, number of steps, and save output in memory
		CTrajectoryProblem* pProblem=(CTrajectoryProblem*)pTask->getProblem();
		pProblem->setModel(pModel);
		pTask->setScheduled(true);
		pProblem->setStepNumber(numSteps);
		pProblem->setDuration(endTime-startTime);
		pDataModel->getModel()->setInitialTime(startTime);
		pProblem->setTimeSeriesRequested(true);
		try
		{
			pTask->initialize(CCopasiTask::OUTPUT_COMPLETE, pDataModel, NULL);
			pTask->process(true);
			pTask->restore();
		}
		catch(...)
		{
			std::cerr << "Error. Running the simulation failed." << std::endl;
			// check if there are additional error messages
			if (CCopasiMessage::size() > 0)
			{
				// print the messages in chronological order
				std::cerr << CCopasiMessage::getAllMessageText(true);
			}
			pTask = 0;
		}
	}
	
	if (pTask)
	{
		const CTimeSeries & timeSeries = pTask->getTimeSeries();
		int rows = timeSeries.getRecordedSteps(), cols = timeSeries.getNumVariables();
		int i,j;
	
		tc_matrix output = tc_createMatrix(rows, cols);
	
		for (j=0; j < cols; ++j)
			tc_setColumnName( output, j, timeSeries.getTitle(j).c_str()  );
	
		for (i=0; i < rows; ++i)
			for (j=0; j < cols; ++j)
				tc_setMatrixValue( output, i, j, timeSeries.getConcentrationData(i,j) );
	
		return output;
	}*/
	return tc_createMatrix(0,0);
}

tc_matrix getSteadyState(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getJacobian(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getEigenvalues(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getUnscaledElasticities(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getUnscaledConcentrationCC(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getUnscaledFluxCC(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getScaledElasticities(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getScaledConcentrationCC(copasi_model model)
{
	return tc_createMatrix(0,0);
}

tc_matrix getScaledFluxCC(copasi_model model)
{
	return tc_createMatrix(0,0);
}
