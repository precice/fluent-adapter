; create scheme variables if they don't already exist
(if (not (rp-var-object 'udf/convergence))
   (rp-var-define 'udf/convergence 1 'int #f)
)
(if (not (rp-var-object 'udf/ongoing))
   (rp-var-define 'udf/ongoing 1 'int #f)
)
(if (not (rp-var-object 'udf/iterate))
   (rp-var-define 'udf/iterate 0 'int #f)
)
(if (not (rp-var-object 'udf/checkpoint))
   (rp-var-define 'udf/checkpoint 0 'int #f)
)
(if (not (rp-var-object 'udf/config-location))
   (rp-var-define 'udf/config-location "./precice-config.xml" 'string #f)
)
(if (not (rp-var-object 'solve/dt))
   (rp-var-define 'solve/dt "0.5" 'string #f)
)

; assign values to variables created
(rpsetvar 'udf/convergence 1)
(rpsetvar 'udf/ongoing 1)
(rpsetvar 'udf/iterate 0)
(rpsetvar 'udf/checkpoint 0)

;;; USER CAN EDIT THESE ;;;
(rpsetvar 'udf/config-location "./precice-config.xml")
(rpsetvar 'solve/dt "0.5")
;;;;;;;;;;;;;;;;;;;;;;;;;;;

; initialize flow field
(rpsetvar 'dynamesh/update-in-timestep/residual-criterion -1.0)
(ti-menu-load-string "solve/initialize/initialize-flow")
