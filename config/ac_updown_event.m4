##*****************************************************************************
## $Id: ac_updown_event.m4,v 1.1.2.1 2006-10-17 06:18:12 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_UPDOWN_EVENT],
[
  AC_MSG_CHECKING([for whether to build updown event module])
  AC_ARG_WITH([slurm-state],
    AC_HELP_STRING([--with-slurm-state], [Build updown event module]),
    [ case "$withval" in
        no)  ac_updown_event_test=no ;;
        yes) ac_updown_event_test=yes ;;
        *)   ac_updown_event_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_updown_event_test=yes}])

  if test "$ac_updown_event_test" = "yes"; then
     AC_DEFINE([WITH_UPDOWN_EVENT], [1], [Define if you want the updown_event module.])
     MANPAGE_UPDOWN_EVENT=1
     ac_with_updown_event=yes
  else
     MANPAGE_UPDOWN_EVENT=0
     ac_with_updown_event=no
  fi
 
  AC_SUBST(MANPAGE_UPDOWN_EVENT)
])