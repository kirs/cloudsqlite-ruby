#include "cloudsqlite.h"

#include "blockcachevfs.h"
#include "sqlite3.h"


#define CS_STORAGE "azure?emulator=127.0.0.1:10000"
#define CS_ACCOUNT "devstoreaccount1"
#define CS_KEY "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="

VALUE rb_mRCEE;
VALUE rb_mSystem;
VALUE rb_cSystemExtension;
static int csExecCb(
  void *pCtx,
  int nStr,
  char **azStr,
  char **azCol
){
  if( azStr ){
    int ii;
    const char *zSep = "";
    for(ii=0; ii<nStr; ii++){
      fprintf(stdout, "%s%s", zSep, azStr[ii] ? azStr[ii] : "(null)");
      zSep = "|";
    }
    fprintf(stdout, "\n");
  }
  return 0;
}

static int csAuthCb(
  void *pCtx,
  const char *zStorage,
  const char *zAccount,
  const char *zContainer,
  char **pzAuthToken
){
  *pzAuthToken = sqlite3_mprintf("%s", CS_KEY);
  return (*pzAuthToken) ? SQLITE_OK : SQLITE_NOMEM;
}

/*
** Open a VFS that uses directory zDir as its cache directory. Then attach
** container zCont. Next, open an SQLite handle on path zPath using the new
** VFS and execute SQL script zSql.
*/
static int cloudsql(
  const char *zDir,               /* Directory to use for blockcachevfs cache */
  const char *zCont,              /* Container to attach */
  const char *zPath,              /* Path to open */
  const char *zSql                /* SQL to execute */
){
  int rc = SQLITE_OK;             /* Error code */
  char *zErr = 0;                 /* Error message */
  sqlite3_bcvfs *pVfs = 0;        /* VFS handle */
  sqlite3 *db = 0;                /* Database handle open on zPath */

  /* Create a VFS object. Directory zDir must already exist. If it exists
  ** and there is a daemon running in that directory, the new VFS connects
  ** to the daemon for read-only access. Or, if there is no such daemon,
  ** the new VFS will provide read-write daemonless access.  */
  rc = sqlite3_bcvfs_create(zDir, "myvfs", &pVfs, &zErr);

  /* Check if this is a daemon VFS or not */
  if( rc==SQLITE_OK ){
    if( sqlite3_bcvfs_isdaemon(pVfs) ){
      printf("VFS is using a daemon\n");
    }else{
      printf("VFS is in daemonless mode\n");
    }
  }

  /* Configure the authorization callback. */
  if( rc==SQLITE_OK ){
    sqlite3_bcvfs_auth_callback(pVfs, 0, csAuthCb);
  }

  /* Attach the container. Specify the SQLITE_BCV_ATTACH_IFNOT flag so that
  ** it is not an error if the container is already attached.
  **
  ** There are two reasons the container might already be attached, even
  ** though the VFS was only just created. Firstly, if this VFS is connected
  ** to a running daemon process, then some other client may have already
  ** attached the container to the daemon. Secondly, VFS objects store their
  ** state in the cache directory so that if they are restarted, all
  ** containers are automatically reattached. So if this (or some other
  ** blockcachevfs application) has run before specifying the same
  ** cache directory, the container may already be attached.  */
  if( rc==SQLITE_OK ){
    rc = sqlite3_bcvfs_attach(pVfs, CS_STORAGE, CS_ACCOUNT, zCont, 0,
        SQLITE_BCV_ATTACH_IFNOT, &zErr
    );
  }

  /* Open a database handle on a cloud database. */
  if( rc==SQLITE_OK ){
    rc = sqlite3_open_v2(zPath, &db, SQLITE_OPEN_READWRITE, "myvfs");
    if( rc!=SQLITE_OK ){
      zErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  /* Enable the virtual table interface. */
  if( rc==SQLITE_OK ){
    rc = sqlite3_bcvfs_register_vtab(db);
  }

  /* Execute the provided SQL script. */
  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, zSql, csExecCb, 0, &zErr);
  }

  sqlite3_close(db);
  sqlite3_bcvfs_destroy(pVfs);

  /* Output any error, free any error message and return. */
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "Error: (%d) %s\n", rc, zErr);
  }
  sqlite3_free(zErr);
  return rc;
}


static VALUE
rb_system_extension_class_do_something(VALUE self)
{
  int major, minor, patch;

  yaml_get_version(&major, &minor, &patch);

    const char *zDir = "tmpdir";
    const char *zCont = "anothercontainer";
    const char *zPath = "/anothercontainer/countries";
    const char *zSql = "SELECT * FROM list";
    int result = cloudsql(zDir, zCont, zPath, zSql);

  return rb_sprintf("libyaml version %d.%d.%d", major, minor, patch);
}


void
Init_cloudsqlite(void)
{
  rb_mRCEE = rb_define_module("RCEE");
  rb_mSystem = rb_define_module_under(rb_mRCEE, "System");
  rb_cSystemExtension = rb_define_class_under(rb_mSystem, "Extension", rb_cObject);
  rb_define_singleton_method(rb_cSystemExtension, "do_something",
                             rb_system_extension_class_do_something, 0);
}

