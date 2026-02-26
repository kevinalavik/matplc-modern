

#include "plc-cc"

/*
    plc_pt_t       _plc_pt;  // point's reference
*/

Plc_Point_t::Plc_Point_t(const char *name) {
  _plc_pt = plc_pt_by_name(name);
  if (_plc_pt.valid == 0)
    // TODO: throw exception!
    ;
}


Plc_Point_t::~Plc_Point_t(void) {
  // Nothing to do for now!
}


void Plc_Point_t::operator= (u32 value) {
  write_u32(value);
}


Plc_Point_t::operator u32(void) {
  return read_u32();
}


void Plc_Point_t::update(void) {
  plc_update_pt(_plc_pt);
}


const char * Plc_Point_t::name(void) {
  // returns point's name
  // TODO...
  return NULL;
}


int Plc_Point_t::length(void) {
  return plc_pt_len(_plc_pt);
}


int Plc_Point_t::status(void) {
  return plc_pt_rw(_plc_pt);
}


u32 Plc_Point_t::read_u32 (void) {
  return plc_get(_plc_pt);
}


void Plc_Point_t::write_u32(u32 val) {
  if (plc_set(_plc_pt, val) < 0)
    // throw exception...
    ;
}


f32 Plc_Point_t::read_f32 (void) {
  return plc_get(_plc_pt);
}


void Plc_Point_t::write_f32(f32 val) {
  if (plc_set_f32(_plc_pt, val))
    // throw exception...
    ;
}




Plc_Point_u32::Plc_Point_u32(const char *name) 
  : Plc_Point_t(name)
{
}


Plc_Point_u32::~Plc_Point_u32(void) {
  // nothing to do yet
}


u32 Plc_Point_u32::read (void) {
  return read_u32();
}


void Plc_Point_u32::write(u32 val) {
  write_u32(val);
}






Plc_Point_f32::Plc_Point_f32(const char *name) 
  : Plc_Point_t(name)
{
}


Plc_Point_f32::~Plc_Point_f32(void) {
  // nothing to do yet
}


f32 Plc_Point_f32::read (void) {
  return read_f32();
}


void Plc_Point_f32::write(f32 val) {
  write_f32(val);
}


void Plc_Point_f32::operator= (f32 value) {
  write_f32(value);
}

Plc_Point_f32::operator f32(void) {
  return read_f32();
}






/*
    const char *_module_name;
*/

MatPLC_t::MatPLC_t(char const *module_name, int argc, char **argv) {
  if (plc_init(module_name, argc, argv) < 0)
    // throw exception...
    ;
}


MatPLC_t::~MatPLC_t(void) {
  if (plc_done() < 0)
    // throw exception...
    ;
}


int MatPLC_t::scan_beg(void) {
  if (plc_scan_beg() < 0)
    // throw exception...
    ;
}


int MatPLC_t::scan_end(void) {
  if (plc_scan_end() < 0)
    // throw exception...
    ;
}


const char *MatPLC_t::module_name(void) {
  // returns _module_name
  // TODO...
  return NULL;
}


int MatPLC_t::update(void) {
  if (plc_update() < 0)
    // throw exception...
    ;
}



