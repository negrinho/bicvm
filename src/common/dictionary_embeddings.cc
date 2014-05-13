// File: dictionary_embeddings.cc
// Author: Karl Moritz Hermann (mail@karlmoritz.com)
// Created: 12-05-2014
// Last Update: Tue 13 May 2014 16:32:07 BST

DictionaryEmbeddings::DictionaryEmbeddings() :
  Theta(0,0), D(0,0,0) { theta_ = nullptr; }

DictionaryEmbeddings::~DictionaryEmbeddings() {}

/*
 * Initialises the dictionary embeddings object. If 'init_words' is true, word
 * embeddings will be initialised randomly. If 'create_new_theta' is true, space
 * will be created for the embeddings, otherwise we assume that theta_ already
 * points to allocated memory of the required size.
 */
void DictionaryEmbeddings::init(bool init_words, bool create_new_theta) {
  int word_width = config.word_representation_size;
  int dict_size = getDictSize();

  theta_size_ = word_width * dict_size;
  if (create_new_theta) theta_ = new Real[theta_size_];
  new (&Theta) WeightVectorType(ptr, theta_size_);
  new (&D) WeightMatrixType(ptr, dict_size, word_width);

  if (init_words)
    Theta.setZero(); // just be safe (if we add down there instead of initializing)

  if (init_words) {
    //std::random_device rd;
    //std::mt19937 gen(rd());
    std::mt19937 gen(0);
    std::normal_distribution<> d_snd(0,1); // Matlab randn / standard normal distribution
    for (int i = 0; i < theta_D_size_; ++i)
      Theta_D(i) = 0.1 * d_snd(gen);
  }
}

/*
 * Move the dictionary embeddings to a specific address in space.
 */
void DictionaryEmbeddings::moveToAddress(Real* new_address)
{
  for (int i=0; i<theta_size_; ++i) {
    new_address[i] = theta_[i];
  }
  delete [] theta_;
  theta_ = new_address;
  init(false,false);
}

/*
 * Creates space for the dictionary (deletes whatever there might already be).
 * Used after creating a new dictionary during corpus loading and before
 * applying whatever pretrained embeddings we might want to use.
 */
void RecursiveAutoencoderBase::createTheta(bool random_init)
{
  delete [] theta_; // can probably get rid of this!
  init(random_init,true);
}

/*
 * Copies embeddings from an old dictionary with possibly different indices.
 */
void DictionaryEmbeddings::initFromDict(const DictionaryEmbeddings& rae,
                                        std::map<LabelID, LabelID> n2o_map) {
  for (auto i=0; i< getDictSize(); ++i) {
    D.row(i) = rae.D.row(n2o_map[i]);
  }
}

/*
 * Set the unknown token (D[0] to the average value of all other entries in the
 * dictionary.
 */
void DictionaryEmbeddingsBase::averageUnknownWord() {
  D.row(0) = D.colwise().sum() / (getDictSize() - 1);
}

Real DictionaryEmbeddings::getLambdaCost(Bools l, Lambdas lambdas) {
  Real lcost = 0.0;
  if (l.D)   lcost += lambdas.D   * 0.5 * Theta_D.cwiseProduct(Theta_D).sum();
  return lcost;
}

void DictionaryEmbeddings::addLambdaGrad(Real* theta_data, Bools l, Lambdas
                                         lambdas) {
  if (l.D) {
    WeightVectorType X = WeightVectorType(theta_data,theta_D_size_);
    X += (Theta_D * lambdas.D);
  }
}

void DictionaryEmbeddings::setIncrementalCounts(Counts *counts, Real *&vars, int &number)
{
  vars = theta_;
  counts->Wl  = counts->D + theta_Wl_size_;
  counts->Bl  = counts->Wl  + theta_Bl_size_;
  number = theta_size_;
}


