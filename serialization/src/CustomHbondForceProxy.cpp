/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2010 Stanford University and the Authors.           *
 * Authors: Peter Eastman                                                     *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include "openmm/serialization/CustomHbondForceProxy.h"
#include "openmm/serialization/SerializationNode.h"
#include "openmm/Force.h"
#include "openmm/CustomHbondForce.h"
#include <sstream>

using namespace OpenMM;
using namespace std;

CustomHbondForceProxy::CustomHbondForceProxy() : SerializationProxy("CustomHbondForce") {
}

void CustomHbondForceProxy::serialize(const void* object, SerializationNode& node) const {
    node.setIntProperty("version", 1);
    const CustomHbondForce& force = *reinterpret_cast<const CustomHbondForce*>(object);
    node.setStringProperty("energy", force.getEnergyFunction());
    node.setIntProperty("method", (int) force.getNonbondedMethod());
    node.setDoubleProperty("cutoff", force.getCutoffDistance());
    SerializationNode& perDonorParams = node.createChildNode("PerDonorParameters");
    for (int i = 0; i < force.getNumPerDonorParameters(); i++) {
        perDonorParams.createChildNode("Parameter").setStringProperty("name", force.getPerDonorParameterName(i));
    }
    SerializationNode& perAcceptorParams = node.createChildNode("PerAcceptorParameters");
    for (int i = 0; i < force.getNumPerAcceptorParameters(); i++) {
        perAcceptorParams.createChildNode("Parameter").setStringProperty("name", force.getPerAcceptorParameterName(i));
    }
    SerializationNode& globalParams = node.createChildNode("GlobalParameters");
    for (int i = 0; i < force.getNumGlobalParameters(); i++) {
        globalParams.createChildNode("Parameter").setStringProperty("name", force.getGlobalParameterName(i)).setDoubleProperty("default", force.getGlobalParameterDefaultValue(i));
    }
    SerializationNode& donors = node.createChildNode("Donors");
    for (int i = 0; i < force.getNumDonors(); i++) {
        int p1, p2, p3;
        vector<double> params;
        force.getDonorParameters(i, p1, p2, p3, params);
        SerializationNode& node = donors.createChildNode("Donor").setIntProperty("p1", p1).setIntProperty("p2", p2).setIntProperty("p3", p3);
        for (int j = 0; j < (int) params.size(); j++) {
            stringstream key;
            key << "param";
            key << j+1;
            node.setDoubleProperty(key.str(), params[j]);
        }
    }
    SerializationNode& acceptors = node.createChildNode("Acceptors");
    for (int i = 0; i < force.getNumAcceptors(); i++) {
        int p1, p2, p3;
        vector<double> params;
        force.getAcceptorParameters(i, p1, p2, p3, params);
        SerializationNode& node = acceptors.createChildNode("Acceptor").setIntProperty("p1", p1).setIntProperty("p2", p2).setIntProperty("p3", p3);
        for (int j = 0; j < (int) params.size(); j++) {
            stringstream key;
            key << "param";
            key << j+1;
            node.setDoubleProperty(key.str(), params[j]);
        }
    }
    SerializationNode& exclusions = node.createChildNode("Exclusions");
    for (int i = 0; i < force.getNumExclusions(); i++) {
        int donor, acceptor;
        force.getExclusionParticles(i, donor, acceptor);
        exclusions.createChildNode("Exclusion").setIntProperty("donor", donor).setIntProperty("acceptor", acceptor);
    }
    SerializationNode& functions = node.createChildNode("Functions");
    for (int i = 0; i < force.getNumFunctions(); i++) {
        string name;
        vector<double> values;
        double min, max;
        bool interpolating;
        force.getFunctionParameters(i, name, values, min, max, interpolating);
        SerializationNode& node = functions.createChildNode("Function").setStringProperty("name", name).setDoubleProperty("min", min).setDoubleProperty("max", max).setIntProperty("interpolating", interpolating);
        SerializationNode& valuesNode = node.createChildNode("Values");
        for (int j = 0; j < (int) values.size(); j++)
            valuesNode.createChildNode("Value").setDoubleProperty("v", values[j]);
    }
}

void* CustomHbondForceProxy::deserialize(const SerializationNode& node) const {
    if (node.getIntProperty("version") != 1)
        throw OpenMMException("Unsupported version number");
    CustomHbondForce* force = NULL;
    try {
        CustomHbondForce* force = new CustomHbondForce(node.getStringProperty("energy"));
        force->setNonbondedMethod((CustomHbondForce::NonbondedMethod) node.getIntProperty("method"));
        force->setCutoffDistance(node.getDoubleProperty("cutoff"));
        const SerializationNode& perDonorParams = node.getChildNode("PerDonorParameters");
        for (int i = 0; i < (int) perDonorParams.getChildren().size(); i++) {
            const SerializationNode& parameter = perDonorParams.getChildren()[i];
            force->addPerDonorParameter(parameter.getStringProperty("name"));
        }
        const SerializationNode& perAcceptorParams = node.getChildNode("PerAcceptorParameters");
        for (int i = 0; i < (int) perAcceptorParams.getChildren().size(); i++) {
            const SerializationNode& parameter = perAcceptorParams.getChildren()[i];
            force->addPerAcceptorParameter(parameter.getStringProperty("name"));
        }
        const SerializationNode& globalParams = node.getChildNode("GlobalParameters");
        for (int i = 0; i < (int) globalParams.getChildren().size(); i++) {
            const SerializationNode& parameter = globalParams.getChildren()[i];
            force->addGlobalParameter(parameter.getStringProperty("name"), parameter.getDoubleProperty("default"));
        }
        const SerializationNode& donors = node.getChildNode("Donors");
        vector<double> params(force->getNumPerDonorParameters());
        for (int i = 0; i < (int) donors.getChildren().size(); i++) {
            const SerializationNode& donor = donors.getChildren()[i];
            for (int j = 0; j < (int) params.size(); j++) {
                stringstream key;
                key << "param";
                key << j+1;
                params[j] = donor.getDoubleProperty(key.str());
            }
            force->addDonor(donor.getIntProperty("p1"), donor.getIntProperty("p2"), donor.getIntProperty("p3"), params);
        }
        const SerializationNode& acceptors = node.getChildNode("Acceptors");
        params.resize(force->getNumPerAcceptorParameters());
        for (int i = 0; i < (int) acceptors.getChildren().size(); i++) {
            const SerializationNode& acceptor = acceptors.getChildren()[i];
            for (int j = 0; j < (int) params.size(); j++) {
                stringstream key;
                key << "param";
                key << j+1;
                params[j] = acceptor.getDoubleProperty(key.str());
            }
            force->addAcceptor(acceptor.getIntProperty("p1"), acceptor.getIntProperty("p2"), acceptor.getIntProperty("p3"), params);
        }
        const SerializationNode& exclusions = node.getChildNode("Exclusions");
        for (int i = 0; i < (int) exclusions.getChildren().size(); i++) {
            const SerializationNode& exclusion = exclusions.getChildren()[i];
            force->addExclusion(exclusion.getIntProperty("donor"), exclusion.getIntProperty("acceptor"));
        }
        const SerializationNode& functions = node.getChildNode("Functions");
        for (int i = 0; i < (int) functions.getChildren().size(); i++) {
            const SerializationNode& function = functions.getChildren()[i];
            const SerializationNode& valuesNode = function.getChildNode("Values");
            vector<double> values;
            for (int j = 0; j < (int) valuesNode.getChildren().size(); j++)
                values.push_back(valuesNode.getChildren()[j].getDoubleProperty("v"));
            force->addFunction(function.getStringProperty("name"), values, function.getDoubleProperty("min"), function.getDoubleProperty("max"), (bool) function.getIntProperty("interpolating"));
        }
        return force;
    }
    catch (...) {
        if (force != NULL)
            delete force;
        throw;
    }
}
