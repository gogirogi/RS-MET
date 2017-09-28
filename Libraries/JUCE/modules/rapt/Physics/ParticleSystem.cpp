template<class T>
rsParticleSystem<T>::rsParticleSystem(int numParticles)
{
  setNumParticles(numParticles);
}

template<class T>
void rsParticleSystem<T>::setNumParticles(int newNumParticles)
{
  particles.resize(newNumParticles);
  forces.resize(newNumParticles);
  initialPositions.resize(newNumParticles);  
  initialVelocities.resize(newNumParticles);
}

template<class T>
rsVector3D<T> rsParticleSystem<T>::getForceBetween(const rsParticle<T>& p1, const rsParticle<T>& p2)
{
  // precomputations:
  rsVector3D<T> r = p2.pos - p1.pos;    // vector pointing from p1 to p2
  T r2  = r.getSquaredEuclideanNorm();  // squared distance between p1 and p2 == |r|^2
  T r2i = 1 / r2;                       // reciprocal of r2 - used as multiplier in various places
  r *= sqrt(r2i);                       // r is now normalized to unit length

                                        // compute the 3 forces:
  rsVector3D<T> f;
  f += r2i*cG * p1.mass   * p2.mass   * r;                                // gravitational force
  f -= r2i*cE * p1.charge * p2.charge * r;                                // electric force
  f += r2i*cM * p1.charge * p2.charge * cross(p1.vel, cross(p2.vel, r));  // magnetic force
  return f;  

  // see here for the force equations (especially magnetic):
  // https://physics.stackexchange.com/questions/166318/magnetic-force-between-two-charged-particles
  // http://teacher.nsrl.rochester.edu/phy122/Lecture_Notes/Chapter30/chapter30.html

  // todo: we need some precautions for cases when the p1 and p2 are (almost) at the same position
  // we get division by zero in such cases (because r2 becomes 0)

  // todo: check, if the magnetic force law has the correct sign (later in the stackexchange 
  // discussion, there's a formula that has the cross product in different order)
  // ...we should really check for all formulas, if we compute the forces on p1 due to p2 (as 
  // desired) or the other way around (which is the same value with negative sign)

  // maybe allow for (non-physical) general inverse power force laws instead of the usual inverse
  // square law
}

template<class T>
void rsParticleSystem<T>::updateForces()
{
  size_t N = particles.size();
  size_t i, j;

  for(i = 0; i < N; i++)
    forces[i] = 0;

  //// naive:
  //for(i = 0; i < N; i++){
  //  for(j = 0; j < N; j++){
  //    if(i != j) forces[i] += getForceBetween(particles[i], particles[j]); }}

  // optimized, using force(j, i) = -force(i, j):
  for(i = 0; i < N; i++){
    for(j = i+1; j < N; j++){
      rsVector3D<T> f = getForceBetween(particles[i], particles[j]);
      forces[i] += f;
      forces[j] -= f; }}

  // todo: test, if both loops give same results (up to roundoff error)
}

template<class T>
void rsParticleSystem<T>::updateVelocities()
{
  for(size_t i = 0; i < particles.size(); i++)
    particles[i].vel += forces[i] / particles[i].mass;
}

template<class T>
void rsParticleSystem<T>::updatePositions()
{
  for(size_t i = 0; i < particles.size(); i++)
    particles[i].pos += stepSize * particles[i].vel;
}

template<class T>
void rsParticleSystem<T>::reset()
{
  for(size_t i = 0; i < particles.size(); i++) {
    particles[i].pos = initialPositions[i];
    particles[i].vel = initialVelocities[i]; }
}