#include <bits/stdc++.h>
#define XX first
#define YY second
#define EPS 1e-9
#define MIN_DENIABILITY 5
#define MIN_CHANGE 0.5
using namespace std;
typedef pair<int, int> pii;

bool checkBit(int mask, int pos){
	return ((mask & (1<<pos))!=0);
}

int setBit(int mask, int pos){
	return (mask | (1<<pos));
}

struct DiscreteAttribute{
	string featureName;
	int domainSize;
	map <string, int> domainValue;
	int getValue(string str){
		if(domainValue.find(str)==domainValue.end()) return -1;
		return domainValue[str];
	}
	
	void sanityCheck(){
		set <int> values;
		set <string> domains;
		for(auto pp: domainValue){
			assert(domains.find(pp.XX)==domains.end());
			assert(values.find(pp.YY)==values.end());
			domains.insert(pp.XX);
			values.insert(pp.YY);
			
			assert(pp.YY>=1 && pp.YY<=domainSize);
		}
		assert((int)domains.size()==domainSize);
		assert((int)values.size()==domainSize);
	}
};

struct ContinuousAttribute{
	string featureName;
};

struct DataTuple{
	vector <int> categoricalValue;
	vector <double> continuousValue;
};

struct Database{
	int targetAttribute;
	int numberOfAttributes;
	int numberOfTuples;
	vector <DiscreteAttribute> categoricalFeatures;
	vector <ContinuousAttribute> continuousFeatures;
	vector <pii> featureIndex; //featureIndex[x]=(0, i) indicates the x-th feature overall is the i-th categorical attribute
							   //featureIndex[x]=(1, i) indicates the x-th feature overall is the i-th continuous attribute
	vector <DataTuple> Dataset;
	
	void sanityCheck(){
		assert(((int)featureIndex.size())==numberOfAttributes);
		assert(((int)categoricalFeatures.size()+(int)continuousFeatures.size())==(int)featureIndex.size());
		assert(featureIndex[targetAttribute].XX==0);
		assert(numberOfTuples==(int)Dataset.size());
		
		int expectedNoOfCategoricalFeatures=0;
		int expectedNoOfContinuousFeatures=0;
		for(auto p: featureIndex){
			if(p.XX==0) expectedNoOfCategoricalFeatures++;
			else{
				assert(p.XX==1);
				expectedNoOfContinuousFeatures++;
			}
		}
		assert(expectedNoOfCategoricalFeatures==(int)categoricalFeatures.size());
		assert(expectedNoOfContinuousFeatures==(int)continuousFeatures.size());
		
		for(auto att: categoricalFeatures){
			att.sanityCheck();
		}
	}
	
	void print(){
		for(auto tuple: Dataset){
			for(auto p: featureIndex){
				if(p.XX==0){
					cout<<tuple.categoricalValue[p.YY]<<" ";
				}
				else{
					cout<<tuple.continuousValue[p.YY]<<" ";
				}
			}
			cout<<endl;
		}
	}
	
}db;


struct DataPartition{
	vector <int> projection;
};

DataPartition discreteSelection(DataPartition part, int featurePos, int targetValue){
	assert(db.featureIndex[featurePos].XX==0);
	int index=db.featureIndex[featurePos].YY;
	
	DataPartition resultingPartition;
	
	for(auto tupleIndex: part.projection){
		if(db.Dataset[tupleIndex].categoricalValue[index]==targetValue){
			resultingPartition.projection.push_back(tupleIndex);
		}
	}
	
	return resultingPartition;
}


DataPartition continuousSelection(DataPartition part, int featurePos, bool careAboutLowerBound, double lowerBound, bool careAboutUpperBound, double upperBound){  /*Upper bound and 
																																							* lower bounds are 
																																							* inclusive*/	
	assert(db.featureIndex[featurePos].XX==1);
	int index=db.featureIndex[featurePos].YY;
	
	DataPartition resultingPartition;
	
	for(auto tupleIndex: part.projection){
		if(db.Dataset[tupleIndex].continuousValue[index]>=lowerBound || !careAboutLowerBound){
			if(db.Dataset[tupleIndex].continuousValue[index]<=upperBound || !careAboutUpperBound){
				resultingPartition.projection.push_back(tupleIndex);
			}
		}
	}
	
	return resultingPartition;
}

void takeInput(){
	scanf("%d", &db.numberOfTuples);
	scanf("%d", &db.numberOfAttributes);
	scanf("%d", &db.targetAttribute);
	string name;
	string value;
	int isContinuous;
	int sz;
	for(int i=0; i<db.numberOfAttributes; i++){
		cin>>name;
		scanf("%d", &isContinuous);
		if(isContinuous){
			sz=(int)db.continuousFeatures.size();
			ContinuousAttribute conAtt;
			conAtt.featureName=name;
			db.continuousFeatures.push_back(conAtt);
		}
		else{
			sz=(int)db.categoricalFeatures.size();
			DiscreteAttribute disAtt;
			disAtt.featureName=name;
			scanf("%d", &disAtt.domainSize);
			for(int i=1; i<=disAtt.domainSize; i++){
				cin>>value;
				disAtt.domainValue[value]=i;
			}
			db.categoricalFeatures.push_back(disAtt);
		}
		db.featureIndex.push_back(pii(isContinuous, sz));
	}
	double val;
	for(int i=0; i<db.numberOfTuples; i++){
		DataTuple tuple;
		for(int j=0; j<db.numberOfAttributes; j++){
			if(db.featureIndex[j].XX==0){
				cin>>value;
				tuple.categoricalValue.push_back(db.categoricalFeatures[db.featureIndex[j].YY].getValue(value));
			}
			else{
				cin>>val;
				tuple.continuousValue.push_back(val);
			}
		}
		db.Dataset.push_back(tuple);
	}
}

vector <double> getVector(DataPartition part){
	vector <int> count;
	int totalSize=part.projection.size();
	for(int i=0; i<db.categoricalFeatures[db.featureIndex[db.targetAttribute].YY].domainSize; i++){
		count.push_back(0);
	}
	for(auto ind: part.projection){
		count[db.Dataset[ind].categoricalValue[db.featureIndex[db.targetAttribute].YY]-1]++;
	}
	
	vector <double> P;
	
	for(auto c: count){
		P.push_back((c*1.0)/(totalSize*1.0));
	}
	
	return P;
}

double getDeniability(DataPartition part){
	if(part.projection.size()==0) return 100.0;
	vector <double> P = getVector(part);
	double sum=0.0;
	for(auto p: P){
		if(p<EPS) continue;
		sum-=p*log2(p);
		//cout<<sum<<endl;
	}
	sum/=(-1.0*log2(1.0/(P.size()*1.0)));
	return sum*100.0;
}

double getDistance(DataPartition part1, DataPartition part2){
	vector <double> x=getVector(part1);
	vector <double> y=getVector(part2);
	
	assert(x.size()==y.size());
	double sum=0.0;
	for(int i=0; i<(int)x.size(); i++){
		sum+=(x[i]-y[i])*(x[i]-y[i]);
	}
	sum/=(x.size()*1.0);
	return sum*100.0;
}


pair <bool, double> discreteSelectionMetric(DataPartition part, int featureIndex){
	assert(db.featureIndex[featureIndex].XX==0);
	double sum=0.0;
	DataPartition tempPartition;
	for(int i=1; i<=db.categoricalFeatures[db.featureIndex[featureIndex].YY].domainSize; i++){
		tempPartition=discreteSelection(part, featureIndex, i);
		if(getDeniability(tempPartition)<MIN_DENIABILITY){
			return {false, 0.0};
		}
		sum+=(1.0*tempPartition.projection.size())*getDistance(part, tempPartition);
	}
	sum/=(1.0*part.projection.size());
	return {true, sum};
}

vector <pair <bool, double> > dp;
vector <int> nxt;
vector <pair <double, int> > ara;
vector <double> splittingPoints;

pair <bool, double> continuousSelectionMetric(DataPartition part, int featureIndex, bool getSplittingPoints){
	assert(db.featureIndex[featureIndex].XX==1);
	ara.clear();
	dp.clear();
	nxt.clear();
	
	int pos=db.featureIndex[featureIndex].YY;
	for(auto ind: part.projection){
		ara.push_back({db.Dataset[ind].continuousValue[pos], ind});
	}
	sort(ara.begin(), ara.end());
	
	
	for(int i=0; i<=(int)ara.size(); i++){
		dp.push_back({false, -1.0});
		nxt.push_back(-1);
	}
	
	dp[ara.size()]={true, 0};
	
	DataPartition subPart;
	
	for(int i=ara.size()-1; i>=0; i--){
		subPart.projection.clear();
		for(int j=i; j<(int)ara.size(); j++){
			subPart.projection.push_back(ara[j].YY);
			if(j!=((int)ara.size()-1) && ara[j].XX==ara[j+1].XX) continue;
			if(!dp[j+1].XX) continue;
			if(getDeniability(subPart)<MIN_DENIABILITY){
				continue;
			}
			
			if(!dp[i].XX || ((j-i+1)*getDistance(part, subPart)+dp[j+1].YY)>dp[i].YY){
				dp[i]={true, (j-i+1)*getDistance(part, subPart)+dp[j+1].YY};
				nxt[i]=j+1;
			}
		}
	}
	
	if(getSplittingPoints){
		splittingPoints.clear();
		int pos=0;
		while(pos!=(int)(ara.size())){
			assert(nxt[pos]!=-1);
			pos=nxt[pos];
			
			if(pos!=0 && pos!=(int)(ara.size())){
				//cout<<"Pos="<<pos<<endl;
				splittingPoints.push_back((ara[pos].XX+ara[pos-1].XX)/2.0);
			} 
		}
	}
	
	dp[0].YY/=(part.projection.size()*1.0);
	
	return dp[0];
	
}


struct node{
	bool isLeaf;
	int partitionSize;
	int classifyingAttribute;
	vector <double> distribution;
	vector <node*> children;
	vector <double> splittingPoints;
	node(){}
	node(vector <double> vec){
		distribution=vec;
	}
};

node* root;

void buildTree(node* curr, DataPartition part, int attributeMask){
	
	curr->partitionSize=part.projection.size();
	
	if(part.projection.size()==0){
		curr->isLeaf=true;
		return;
	}
	
	curr->distribution=getVector(part);
	
	if(attributeMask==((1<<db.numberOfAttributes)-1)){
		curr->isLeaf=true;
		return;
	}
	
	curr->isLeaf=false;
	
	double mx=0.0;
	curr->classifyingAttribute=-1;
	pair <bool, double> p;
	
	for(int i=0; i<db.numberOfAttributes; i++){
		if(!checkBit(attributeMask, i)){
			if(db.featureIndex[i].XX==0)
				p=discreteSelectionMetric(part, i);
			else
				p=continuousSelectionMetric(part, i, false);
			if(p.XX && p.YY>MIN_CHANGE){
				if(p.YY>mx){
					curr->classifyingAttribute=i;
					mx=p.YY;
				}
			}
		}
	}
	
	
	if(curr->classifyingAttribute==-1){
		curr->isLeaf=true;
		return;
	}
	
	
	DataPartition subPart;
	node* child;
	if(db.featureIndex[curr->classifyingAttribute].XX==0){
		for(int i=1; i<=db.categoricalFeatures[db.featureIndex[curr->classifyingAttribute].YY].domainSize; i++){
			subPart=discreteSelection(part, curr->classifyingAttribute, i);
			child=new node(curr->distribution); 
			curr->children.push_back(child);
			buildTree(child, subPart, setBit(attributeMask, curr->classifyingAttribute));
		}
	}
	else{
		continuousSelectionMetric(part, curr->classifyingAttribute, true);
		subPart=continuousSelection(part, curr->classifyingAttribute, false, 0.0, true, splittingPoints[0]);
		child=new node(curr->distribution);
		curr->children.push_back(child);
		buildTree(child, subPart, setBit(attributeMask, curr->classifyingAttribute));
		curr->splittingPoints=splittingPoints;
		for(int i=0; i<(int)splittingPoints.size(); i++){
			if(i==((int)splittingPoints.size()-1)) subPart=continuousSelection(part, curr->classifyingAttribute, true, splittingPoints[i], false, 0.0);
			else subPart=continuousSelection(part, curr->classifyingAttribute, true, splittingPoints[i], true, splittingPoints[i+1]);
			child=new node(curr->distribution);
			curr->children.push_back(child);
			buildTree(child, subPart, setBit(attributeMask, curr->classifyingAttribute));
		}
	}
}

int getValue(node * curr, DataTuple tuple){
	if(curr->isLeaf){
		int val=rand()%(curr->partitionSize);
		//for(auto d: curr->distribution) cerr<<d<<" ";
		//cerr<<endl;
		for(int i=0; i<(int)curr->distribution.size(); i++){
			//cerr<<val<<" "<<curr->distribution[i]*curr->partitionSize<<"\n";
			if((curr->distribution[i]*curr->partitionSize)>val){ /*cerr<<"Returning"<<endl;*/ return i;}
			val-=(curr->distribution[i]*curr->partitionSize);
		}
		//cerr<<"Returning end"<<endl;
		return (int)curr->distribution.size()-1;
	}
	
	int att=curr->classifyingAttribute;
	if(db.featureIndex[att].XX==0){
		return getValue(curr->children[tuple.categoricalValue[db.featureIndex[att].YY]-1], tuple);
	}
	int cnt=0;
	for(auto p: curr->splittingPoints){
		if(tuple.continuousValue[db.featureIndex[att].YY]<p){
			return getValue(curr->children[cnt], tuple);
		}
		cnt++;
	}
	return getValue(curr->children[cnt], tuple);
}

void writeOutput(){
	cout<<db.numberOfTuples<<endl;
	cout<<db.numberOfAttributes<<endl;
	cout<<db.targetAttribute<<endl;
	for(int i=0; i<(int)db.featureIndex.size(); i++){
		if(db.featureIndex[i].XX==0){
			cout<<db.categoricalFeatures[db.featureIndex[i].YY].featureName<<" "<<0<<" "<<db.categoricalFeatures[db.featureIndex[i].YY].domainSize<<endl;
		}
		else{
			cout<<db.continuousFeatures[db.featureIndex[i].YY].featureName<<" "<<1<<endl;
		}
	}
	for(int i=0; i<db.numberOfTuples; i++){
		for(int j=0; j<(int)db.featureIndex.size(); j++){
			if(j) cout<<" ";
			if(j==db.targetAttribute){
				cout<<getValue(root, db.Dataset[i])+1;
			}
			else if(db.featureIndex[j].XX==0){
				cout<<db.Dataset[i].categoricalValue[db.featureIndex[j].YY];
			}
			else{
				cout<<db.Dataset[i].continuousValue[db.featureIndex[j].YY];
			}
		}
		cout<<endl;
	}
}

int main(){
	srand(time(NULL));
	freopen("nursery.txt", "r", stdin);
	freopen("nurseryout.txt", "w", stdout);
	takeInput();
	
	
	DataPartition init;
	for(int i=0; i<db.numberOfTuples; i++){
		init.projection.push_back(i);
	}
	
	root=new node();
	buildTree(root, init, setBit(0, db.targetAttribute));
	writeOutput();
	
	return 0;
}
