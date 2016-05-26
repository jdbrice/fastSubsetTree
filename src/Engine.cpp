

#include "XmlConfig.h"
#include "ChainLoader.h"
#include "Utils.h"
using namespace jdb;

#include <iostream>
#include <exception>

#include "TChain.h"
#include "TFile.h"
#include "TMath.h"

int main( int argc, char* argv[] ) {

	if ( argc >= 2 ){

	} else {
		cout << "Please provide a config file." << endl;
		return 0;
	}

	XmlConfig config( argv[ 1 ] );

	TChain * chain = new TChain( config.getString( "input:tree", "MuDst" ).c_str() );
	ChainLoader::load( chain, config.getString("input:url", "./").c_str(), config.getInt( "input:maxFiles", -1 ) );

	cout << "Calcing Split" << endl;
	int totalTrees = chain->GetNtrees();
	int splitEvery = config.getInt( "output:split", 0 ); // zero = don't split
	int passes = 1;
	if ( splitEvery > 0 ){
		passes = TMath::Ceil((double)totalTrees / (double)splitEvery);
	} else {
		splitEvery = totalTrees;
	}

	delete chain;
	for (  int i = 0; i < passes; i++ ){
		cout << "Pass " << i << endl;
		TChain * chain = new TChain( config.getString( "input:tree", "MuDst" ).c_str() );
		ChainLoader::loadRange( chain, config.getString("input:url", "./"),
								splitEvery * i, splitEvery *( i + 1) );


		chain->SetBranchStatus( "*", 0 );

		vector<string> branches = config.getStringVector( "KeepBranches" );
		for ( uint i = 0; i < branches.size(); i++ ){
			chain->SetBranchStatus( branches[ i ].c_str(), 1 );
			cout << "[Keeping Branch]    " << branches[ i ] << endl;
		}

		string outName = config.getString( "output:prefix" ) + ts( i ) + config.getString( "output:suffix" );


		TFile *newFile = new TFile( outName.c_str() ,"recreate");
		newFile->SetCompressionLevel( 9 );
		TTree *newTree = chain->CloneTree( -1, "fast");

		newFile->Write();
		delete newFile;
		delete chain;

	}	

	return 0;
}